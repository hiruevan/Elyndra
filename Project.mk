# General name + desc
NAME = "Elyndra"
DESCRIPTION = "Elyndra: The Sevenfold"

# Build information
TARGET = "Elyndra" # Build name
SRCDIR = /src # Location of all C files
# INCLUDE = /include # Locations of H files (Not being used in Elyndra right now)
SRC = main.c # Entrance script

# Directories
BIN = /bin # Binary location
BUILD = /build # Build dump
EXPORT = /export # Final binary export

# Resources (and files found in /resources)
RESOURCES = /resources # Where all the local cbuild resource files (used by the C files) can be found

KEYBINDS = keybinds.cfg
ICON = icon.ico

# Platforms
PLATFORMS = cal win lnx-x86_64 lnx-aarch64 webp gc # wii # All the platforms that this is designed to run on (runs all of these with -all)