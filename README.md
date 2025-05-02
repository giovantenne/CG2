# CryptoGadgets v2

This is a **free and open source project** that let you check the bitcoin price in real time with a small piece of hardware.

Unlike similar devices this one uses WebSockets and updates the display real-time, several time per second!

I first created these objects for myself, playing with 3D modeling and ESP32 software development.

[![See the details here](https://www.datocms-assets.com/56675/1680892734-btc-edited-edited.png?fm=webp&w=610)](https://www.cryptogadgets.net/CG2/cryptocurrency-ticker-v2-real-time-bitcoin-btc-price)


## Features

- WiFi powered. No PC needed
- True real-time price
- Display up to 9 digits
- Adjustable display brightness
- 24h price change
- Support for more than 600 pairs
- Upgradable firmware

## User manual

This is a stand-alone device. This means that you don’t need a PC  to get it running and that you can simply power it through the micro-usb port. Before doing so  you will need to connect it to your WiFi network by following these simple steps:

- Power-up your device. When setting it up for the first time, and everytime it is not able to connect to an existing WiFi network, the device will start in “access-point/captive-portal" mode and it will broadcast the WiFi SSID “ToTheMoon”.
- Connect your phone/computer to the “ToTheMoon” WiFi network using the following password: 12345678
- Your phone/computer should prompt you with a login/welcome page. If not you can simply browse to the following address: http://172.217.28.1. You should then be able to see and access the device setup interface.
- On the menu click “Configure new AP”. You should then be presented with a list of existing WiFi networks.
- Click on your network SSID name, enter your passphrase and click “Apply”.


After the first setup, may  you want to change your currency pair, adjust the number of decimal digits or change the display brightness, you can simply browse to the address shown on the display during the booting process after the logo. Before doing so, please ensure  to be connected to the same WiFi network.

You also can easily update the device firmware by downloading the latest version, if needed, from the following link: https://github.com/giovantenne/CG2/releases

You can check the firmware version you are currently running  by simply  looking at the display during the booting process, below the bitcoin logo.


## Unboxing and setup video
[![See the video here](https://www.datocms-assets.com/56675/1708071548-cryptocurrency-ticker-v2-real-time-bitcoin-btc-price.png?auto=format&w=610)](https://www.youtube.com/watch?v=KbjArYrW6Es)


## Requirements
- TTGO T-Display - [here](https://www.lilygo.cc/products/lilygo%C2%AE-ttgo-t-display-1-14-inch-lcd-esp32-control-board)
- 3D BOX - [here](stl/)

## Download and load the firmware

Easiest way to flash firmware. Build your own CGv2 using the following firmware flash tool:

- Download the last firmware from the _Releases_ page: https://github.com/giovantenne/CG2/releases`
- Download the [partitions.bin](https://github.com/giovantenne/CG2/blob/master/bin/partitions.bin) file
- Download the [bootloader.bin](https://github.com/giovantenne/CG2/blob/master/bin/bootloader.bin) file
- Use the nice [ESP Web Tool Flash](https://esp.huhn.me/) tool to Flash your device firmware through your browser  (recommend via Google Chrome incognito mode)
- Connect your device and adjust the table as for the image below and click _PROGRAM_:
![ESP Web Tool Flash table](https://github.com/giovantenne/CG2/blob/master/bin/ESPWebTool.png)

## Build from source and load the firmware
- Clone this repository `git clone https://github.com/giovantenne/CG2`
- Install PlatformIO Core (https://platformio.org/install/cli)
- Connect the board via USB
- Run `cd CG2 && pio run -t upload`

## Donations/Project contributions
If you would like to contribute and help dev team with this project you can send a donation to the following LN address ⚡`cg@pos.btcpayserver.it`⚡ or on-chain `bc1qdx6r7z2c2dtdfa2tn9a2u4rc3g5myyfquqq97x`

If you want to order a fully assembled multipurpose V2 or V3 devices you can contribute to my job at https://www.cryptogadgets.net

Enjoy!
