# ESP32 E-Paper Inspirational Quotes Display (GDEW029Z10)

This project creates an e-paper display that periodically shows inspirational quotes and their authors. It's designed for low-power consumption, utilizing the ESP32's deep sleep capabilities and waking up on a set interval or via a button press to refresh the displayed quote.

## Features

* **Random Quote Display:** Fetches and displays a new random quote from a predefined collection.
* **Low Power Consumption:** Utilizes ESP32's deep sleep mode, waking up only for updates or button presses.
* **E-Paper Display:** Designed for a 2.9-inch GDEW029Z10 (SSD1680, black/white/red) e-paper display.
* **Configurable Update Interval:** The display refreshes automatically after a set period (default: 5 hours).
* **Manual Refresh Button:** A dedicated button allows for immediate refresh of the quote.
* **Ghosting Prevention:** Periodically clears the screen to mitigate e-paper ghosting effects.
* **Word Wrapping:** Implements custom word wrapping to ensure quotes fit neatly within the display area without splitting words.

## Hardware Requirements

* **ESP32 Development Board:** (e.g., ESP32 DevKitC)
* **2.9'' GDEW029Z10 E-Paper Display:** (SSD1680 controller, Black/White/Red)
* **Push Button:** For manual refresh.
* **Breadboard and Jumper Wires** (optional, for prototyping)

## Pin Configuration

The following pin connections are used in the code:

| Component             | ESP32 Pin |
| :-------------------- | :-------- |
| E-Paper CS            | GPIO 5    |
| E-Paper DC            | GPIO 0    |
| E-Paper RST           | GPIO 2    |
| E-Paper BUSY          | GPIO 15   |
| E-Paper SCK           | GPIO 18   |
| E-Paper MOSI          | GPIO 23   |
| Manual Refresh Button | GPIO 4    |

## Software Requirements

* **PlatformIO IDE:** Recommended for building and uploading the code.
* **Arduino IDE:** Can also be used with the ESP32 board package installed.