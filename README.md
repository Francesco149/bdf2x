pixel-perfect upscales bdf bitmap fonts

![](http://hnng.moe/f/ZsR)
![](http://hnng.moe/f/Zsa)

rationale
-----------------------------------------------------------
I love the pixel art aesthetics of small bitmap
fonts, but they're way too small for a 1080p screen and
bigger fonts don't look quite as good, so I figured I'd
simply take the small fonts and make every pixel a 2x2
block

for now I've only implemented 2x scaling, and while it's
trivial to add 3x, I'm not gonna bother as this was made
out of necessity. feel free to PR it though!

usage
-----------------------------------------------------------
```sh
curl -O https://raw.githubusercontent.com/Francesco149/bdf2x/master/bdf2x.c
sudo gcc bdf2x.c -o /usr/bin/bdf2x

# copy your desired font and convert it to bdf
cat /usr/share/fonts/X11/misc/ProFont_r400-11.pcf \
| pcf2bdf -o ProFont_r400-11.bdf

bdf2x ProFont_r400-11.bdf > 2xProFont_r400-11.bdf

bdftopcf 2xProFont_r400-11.bdf \
| gzip \
| sudo tee /usr/share/fonts/X11/misc/2xProFont_r400-11.pcf.gz \
> /dev/null

sudo mkfontdir /usr/share/fonts/X11/misc
xset fp rehash

# check your font in xfontsel
```

license
-----------------------------------------------------------
this is free and unencumbered software released into the
public domain.

refer to the attached UNLICENSE or http://unlicense.org/
