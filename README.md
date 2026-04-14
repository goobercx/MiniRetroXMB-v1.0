# MiniRetroXMB

I got tired of having to transfer files back and forth to my PC just to change my RetroArch theme on my Vita. So I built this.

MiniRetroXMB lets you pick colors, choose a wallpaper, preview everything, and save it as a theme — all directly on the Vita. No PC, no FTP, no hassle.

---

## What it does

- Pick an accent color using a color grid (you can also type in a hex code)
- Browse wallpapers from your photo library with thumbnail previews
- Preview how your theme looks before saving it
- Save multiple themes with custom names so you can switch between them
- Apply a theme and launch RetroArch in one button press
- Manage your saved themes — rename, apply, or delete them

---

## What you need

- A hacked PS Vita (HENkaku, h-encore, or Enso — any of them work)
- RetroArch installed on your Vita
- Some images in ux0:picture/ if you want custom wallpapers

---

## Installation

1. Download MiniRetroXMB.vpk from the Releases page
2. Copy it to your Vita using VitaShell (FTP or USB, either works)
3. Press Cross on the VPK in VitaShell to install
4. Open it from your LiveArea

---

## Controls

**Getting around:**

| Button | Where it takes you |
|--------|-------------------|
| L Trigger | Color Picker |
| R Trigger | Background Picker |
| Select | Preview |
| Cross on Preview | Save screen |
| Triangle | Saved Themes |
| START twice | Exit |

**Color Picker:**

| Button | What it does |
|--------|-------------|
| D-Pad | Move around the color grid |
| Left Analog Stick | Adjust brightness |
| Triangle | Type in a hex color code |

**Background Picker:**

| Button | What it does |
|--------|-------------|
| D-Pad | Scroll through images |
| Cross | Select an image |
| Triangle | View file info |
| Select | Go to Themes |

**Save Screen:**

| Button | What it does |
|--------|-------------|
| Triangle | Edit the theme name |
| Cross | Save the theme |
| Square | Save and open RetroArch |
| Circle | Browse saved themes |

**Themes Browser:**

| Button | What it does |
|--------|-------------|
| D-Pad Up/Down | Navigate the list |
| Cross | Apply this theme |
| Square | Delete this theme |
| Circle | Go back |

---

## How it actually works

When you save a theme, MiniRetroXMB edits RetroArch's config file at ux0:data/retroarch/retroarch.cfg and sets the wallpaper path and font color values. Themes get saved to ux0:app/RETROVITA/assets/xmb/[your theme name]/.

Give it about 30 seconds after saving before you panic — writing the config and launching RetroArch takes a moment.

---

## Notes

- This is my first Vita homebrew so be gentle lol
- Tested on firmware 3.60 with Enso
- If RetroArch is not installed the app won't crash, it just won't do anything when you try to launch it


---

Built using VitaSDK and vita2d.

Made by goobercx
