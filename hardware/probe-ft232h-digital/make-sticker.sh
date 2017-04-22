#!/bin/bash

serial="$1"
if [ "$serial" == "" ]; then
    echo "serial not given"
    exit 1
fi

sticker_file="sticker-$serial.png"

# 1.7 inch * 1.0 inch (300 dpi)
convert -units PixelsPerInch -density 300 -size 510x300 canvas:white "$sticker_file"

convert -font Arial -pointsize 48 -fill black -draw "text 10,50 \"8-channel digital probe\"" "$sticker_file" "$sticker_file"
convert -font Arial -pointsize 24 -fill black -draw "text 10,80 \"S/N: $serial\"" "$sticker_file" "$sticker_file"

convert -font Arial -pointsize 36 -fill black -gravity center -draw "text 0,-30 \"ADBUS\"" "$sticker_file" "$sticker_file"
convert -font FreeMono -pointsize 54 -fill black -gravity center -draw "text 0,10 \"7 6 5 4 3 2 1 0\"" "$sticker_file" "$sticker_file"

convert -stroke black -strokewidth 5 -draw "line 10,190 500,190" "$sticker_file" "$sticker_file"

convert -font FreeMono -pointsize 54 -fill black -gravity center -draw "text 0,70 \"⏚ ⏚ ⏚ ⏚ 1 2 3 0\"" "$sticker_file" "$sticker_file"
convert -font Arial -pointsize 36 -fill black -gravity center -draw "text 130,110 \"ACBUS\"" "$sticker_file" "$sticker_file"

