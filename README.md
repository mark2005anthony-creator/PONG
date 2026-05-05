# 🏓 PONG

A classic PONG arcade game clone with **C++ native** (Windows) and **HTML browser** versions.

## 🎮 Features

- **1 Player** — Play against the AI
- **2 Players** — Local multiplayer
- **Clickable menu** — Start, Settings, and Game Over screens
- **Extensive settings** — Customize colors, ball speed, paddle size, AI difficulty, paddle speed, sound, score limit, and timer
- **Auto-saving** — Settings persist in `pong_settings.ini`
- **Fullscreen-friendly** — Responsive scaling for any window size

## 🕹️ Controls

| Key | Action |
|-----|--------|
| **W / S** | Move left paddle |
| **↑ / ↓** | Move right paddle (2P mode) |
| **P** | Pause / Resume |
| **ESC** | Return to menu |

### Settings Keys

| Key | Setting |
|-----|---------|
| **1-4** | Player 1, Player 2, Ball, Score colors |
| **5** | Score to win |
| **6** | Time limit toggle |
| **B** | Ball speed |
| **P** | Paddle size |
| **A** | AI difficulty |
| **K** | Paddle speed |
| **S** | Sound on/off |

## 🖥️ Versions

### C++ Native (`pong.cpp`)
- Windows executable — compile with MinGW:
  ```
  g++ pong.cpp -o pong.exe -lgdi32 -lwinmm -static -O2 -mwindows
  ```
- DirectX-less, uses pure Win32 GDI with double-buffering
- No external dependencies (other than Windows)

### HTML Browser (`pong.html`)
- Open in any modern browser
- No installation required
- Full settings UI with color pickers

## 🚀 Quick Start

1. **C++ version:** Run `pong.exe` (or compile from source)
2. **HTML version:** Open `pong.html` in your browser
3. Click **1 PLAYER** or **2 PLAYERS** to start
4. Press **ESC** for settings

## 📸

```
┌─────────────────────────────────────────────┐
│                  PONG                        │
│                                             │
│          [ 1 PLAYER ]                       │
│          [ 2 PLAYERS ]                      │
│          [ SETTINGS ]                       │
│                                             │
│   W/S - MOVE  |  P - PAUSE  |  ESC - MENU  │
└─────────────────────────────────────────────┘
```
