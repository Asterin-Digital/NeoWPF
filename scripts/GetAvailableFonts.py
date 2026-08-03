import os
import winreg

def list_fonts():
    fonts = []
    key = winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, 
                         r"SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts")
    
    i = 0
    while True:
        try:
            name, value, _ = winreg.EnumValue(key, i)
            fonts.append(name)
            i += 1
        except WindowsError:
            break
    
    winreg.CloseKey(key)
    return sorted(fonts)

if __name__ == "__main__":
    fonts = list_fonts()
    
    print(f"Total fonts: {len(fonts)}\n")
    for i, font in enumerate(fonts, 1):
        print(f"{i}. {font}")