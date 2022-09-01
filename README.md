![Visitor count](https://shields-io-visitor-counter.herokuapp.com/badge?page=EE-modders.Empire-Earth-Discord)
[![Stars](https://img.shields.io/github/stars/EE-modders/Empire-Earth-Discord)](https://github.com/EE-modders/Empire-Earth-Discord/stargazers)
[![Forks](https://img.shields.io/github/forks/EE-modders/Empire-Earth-Discord)](https://github.com/EE-modders/Empire-Earth-Discord/network)
[![Release](https://img.shields.io/github/v/release/EE-modders/Empire-Earth-Discord?label=last%20release%20)](https://github.com/EE-modders/Empire-Earth-Discord/releases/latest)
[![License](https://img.shields.io/github/license/EE-modders/Empire-Earth-Discord?color=brightgreen)](https://github.com/EE-modders/Empire-Earth-Discord/blob/master/LICENSE)
# 🕹️ Empire Earth Discord
A simple DLL, to show your Empire Earth activity on Discord :> \
By [EnergyCube](https://github.com/EnergyCube) for the Empire Earth Community.

## 💡 Features
⏰ Time played
🔍 Detect EE or EE:AoC
💻 [EE Only] Shows detailed activity (Menu/In Game/etc...)
💤 Shows if you are AFK

## 🧾 How to install ?
Download `EEDiscord.dll` [here](https://github.com/EE-modders/Empire-Earth-Discord/releases/latest) and place it in your game directory (next to `Empire Earth.exe` and/or `EE-AOC.exe`) \
Then download the [Discord Game SDK](https://discord.com/developers/docs/game-sdk/sdk-starter-guide) and copy the file `discord_game_sdk.dll` (zip path: `lib\x86\`) in the same folder where you placed `EEDiscord.dll`.

## 🔨 Build
Download the [Discord Game SDK](https://discord.com/developers/docs/game-sdk/sdk-starter-guide) and place all files from `cpp\` in the `discord-files` folder of the project (`EEDiscord/EEDiscord/discord-files`). \
Then place `discord_game_sdk.dll` and `discord_game_sdk.dll.lib` in `discord-files\lib\x86` (and optionally same things for x86_64) and if everything is good, you should be able to build!

# ❤️ Credit
[Dr.MonaLisa](https://github.com/HerMajestyDrMona): For help to setup Discord Game SDK

## 📖 License
[GNU General Public License v3.0](https://github.com/EE-modders/Empire-Earth-Discord/blob/master/LICENSE)
