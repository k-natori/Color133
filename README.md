This project is designed for the PlatformIO environment and targets the GDEP133C02 e-paper display using Arduino. It provides source code, libraries, and configuration files to control and interact with the display.

## Project Structure

- `platformio.ini` — PlatformIO configuration file.
- `src/` — Main source code directory:
  - `GDEP133C02_Arduino.cpp` / `GDEP133C02_Arduino.h`: Arduino version of driver and interface for the GDEP133C02 e-paper display.
  - `html.h`: HTML to upload image via WiFi.
  - `main.cpp`: Main application entry point.
  - `pindefine.h`: Pin definitions for driver board.

## Getting Started

1. **Install PlatformIO**: [PlatformIO Installation Guide](https://platformio.org/install)
2. **Clone this repository**
3. **Open the project in VS Code with the PlatformIO extension**
4. **Connect your hardware** according to the pin definitions in `pindefine.h`
5. **Build and upload** the firmware using PlatformIO commands

## Usage

- Modify `main.cpp` to implement your application logic.
- Use the provided display driver to interact with the GDEP133C02 e-paper display.

