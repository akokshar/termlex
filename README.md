**my *Simple* terminal.**

sudo dnf install vte3-devel gtk3-devel

git clone https://github.com/akokshar/termlex.git

cd termlex

make

sudo make install

**Configure font and colors:**

.Xresources

Termlex*font: PT Mono 12
! special
*.foreground:   #f1ebeb
*.background:   #272822
*.cursorColor:  #f1ebeb

! black
*.color0:       #48483e
*.color8:       #76715e

! red
*.color1:       #dc2566
*.color9:       #fa2772

! green
*.color2:       #8fc029
*.color10:      #a7e22e

! yellow
*.color3:       #d4c96e
*.color11:      #e7db75

! blue
*.color4:       #55bcce
*.color12:      #66d9ee

! magenta
*.color5:       #9358fe
*.color13:      #ae82ff

! cyan
*.color6:       #56b7a5
*.color14:      #66efd5

! white
*.color7:       #acada1
*.color15:      #cfd0c2

