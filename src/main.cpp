// Enable GxEPD2_GFX base class
#define ENABLE_GxEPD2_GFX 0

#include <GxEPD2_3C.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <ESP32Time.h>
#include <quotes.h>

// Pin configuration for ESP32 and 2.9'' GDEW029Z10
#define EPD_CS 5    // Chip Select
#define EPD_DC 0    // Data/Command
#define EPD_RST 2   // Reset
#define EPD_BUSY 15 // Busy
#define EPD_SCK 18  // SPI Clock
#define EPD_MOSI 23 // SPI Data
#define BUTTON_PIN 4 // Button for manual refresh
//#define LED_PIN 22  // LED

// Update interval (seconds): 60 = 1 min
const uint32_t UPDATE_INTERVAL = 18000; // 5 hours

// Display initialization (GDEW029Z10, 2.9-inch, SSD1680, black/white/red)
GxEPD2_3C<GxEPD2_290_C90c, GxEPD2_290_C90c::HEIGHT> display(GxEPD2_290_C90c(EPD_CS, EPD_DC, EPD_RST, EPD_BUSY));

// ESP32 RTC for tracking time
ESP32Time rtc;

// Global variables
int lastQuoteIndex = -1;
volatile bool buttonPressed = false;
int refreshCount = 0; // Track refreshes for ghosting prevention

// Function prototypes
void enterDeepSleep(); 

// Button interrupt handler
void IRAM_ATTR handleButton() {
  buttonPressed = true;
}

// Get a quote from vocab array
void getQuote(String &quote, String &author) {
  int index = random(vocabSize);
  while (index == lastQuoteIndex) {
    index = random(vocabSize);
  }
  lastQuoteIndex = index;
  quote = String((char*)pgm_read_ptr(&vocab[index][1]));
  author = String((char*)pgm_read_ptr(&vocab[index][0]));
}

// Manual word wrapping to prevent word splitting and check height
String wrapText(const String &text, int maxWidth, int maxHeight, const GFXfont *font) {
  display.setFont(font);
  String result = "";
  String currentLine = "";
  int16_t tbx, tby;
  uint16_t tbw, tbh;
  int currentHeight = 0;

  // Split text into words
  int start = 0;
  int len = text.length();
  for (int i = 0; i <= len; i++) {
    if (i == len || text[i] == ' ') {
      String word = text.substring(start, i);
      if (word.length() > 0) {
        String testLine = currentLine + (currentLine.length() > 0 ? " " : "") + word;
        display.getTextBounds(testLine, 0, 0, &tbx, &tby, &tbw, &tbh);
        
        if (tbw <= maxWidth) {
          currentLine = testLine;
        } else {
          if (currentLine.length() > 0) {
            display.getTextBounds(currentLine, 0, 0, &tbx, &tby, &tbw, &tbh);
            if (currentHeight + tbh <= maxHeight) {
              result += currentLine + "\n";
              currentHeight += tbh;
            } else {
              result += "..."; // Truncate if too tall
              return result;
            }
          } else {
            result += word + "\n"; // Word too long for one line
            currentHeight += tbh;
          }
          currentLine = word;
        }
      }
      start = i + 1;
    }
  }

  if (currentLine.length() > 0) {
    display.getTextBounds(currentLine, 0, 0, &tbx, &tby, &tbw, &tbh);
    if (currentHeight + tbh <= maxHeight) {
      result += currentLine;
    } else {
      result += "..."; // Truncate if too tall
    }
  }

  return result;
}

// Display quote and author
void displayQuote(const String &quote, const String &author) {
  Serial.println("Starting displayQuote...");
  display.setRotation(1); // Landscape (296x128)
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Display quote (wrapped, centered)
    display.setFont(&FreeSans9pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setTextWrap(false); // Disable auto word splitting
    String wrappedQuote = wrapText(quote, display.width() - 20, 88, &FreeSans9pt7b); // 10px margin, 108-20=88px height
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(wrappedQuote, 0, 0, &tbx, &tby, &tbw, &tbh);
    uint16_t x = ((display.width() - tbw) / 2) - tbx;
    uint16_t y = 20; // Top margin
    display.setCursor(x, y);
    display.print(wrappedQuote);
    Serial.println("Quote drawn: " + wrappedQuote);

    // Display author (right-aligned, red)
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_RED);
    String authorText = "- " + author;
    display.getTextBounds(authorText, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = display.width() - tbw - 10; // 10px right margin
    y = display.height() - 20; // Near bottom
    display.setCursor(x, y);
    display.print(authorText);
    Serial.println("Author drawn: " + authorText);
  } while (display.nextPage());
  Serial.println("Display update complete.");
  refreshCount++;
}

// Clear screen to reduce ghosting (every 5 refreshes)
void clearScreen() {
  Serial.println("Starting clearScreen...");
  display.setRotation(1); // Landscape
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());
  Serial.println("Screen cleared to reduce ghosting.");
}

void setup() {
  // Initialize serial
  Serial.begin(115200);
  delay(100);
  Serial.println("Starting Inspirational Screen (GDEW029Z10)...");

  //digitalWrite(LED_PIN, HIGH);
  //pinMode(LED_PIN, OUTPUT);

  // Initialize button with interrupt
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);

  // Initialize display
  Serial.println("Initializing display...");
  display.init(115200, true, 50, false); // Match working config
  Serial.println("Display init complete. Color support: " + String(display.epd2.hasColor));

  // Set RTC (May 16, 2025, 09:38, matching 09:38 AM CEST)
  rtc.setTime(0, 38, 9, 16, 5, 2025);

  // Check if woken by button
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 && buttonPressed) {
    Serial.println("Woken by button press");
    buttonPressed = false;
  }

  // Clear screen every 5 refreshes to reduce ghosting
  // Consider increasing this interval, e.g., to 10, if speed is critical
  if (refreshCount % 10 == 0) { // Changed from 5 to 10
    clearScreen();
  }

  // Update quote display
  String quote, author;
  getQuote(quote, author);
  displayQuote(quote, author);

  // Enter deep sleep
  enterDeepSleep();
}

// Enter deep sleep
void enterDeepSleep() {
  Serial.println("Entering deep sleep for " + String(UPDATE_INTERVAL) + " seconds...");
  display.hibernate();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, 0); // Wake on button low
  esp_sleep_enable_timer_wakeup(UPDATE_INTERVAL * 1000000ULL); // Wake after interval
  esp_deep_sleep_start();
}

void loop() {
  // Should not reach here due to deep sleep
  Serial.println("Error: Reached loop!");
  delay(1000);
}