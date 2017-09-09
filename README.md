# StdinImgCnt
##Info
This is a little tool to count imaged piped into it.
It also shows the elapsed time as well as the FPS between frames and
calculates mean values over 20 frames (changeable via define).
It stops reading after a 3 second timeout (also changeable via define).

##Usage
```
cat 1.bmp 2.bmp 3.bmp | ./StdinImgCnt
```
```
cat 1.png 2.png 3.png | ./StdinImgCnt
```
```
seq 1 10000 | xargs -Inone cat foo.bmp | ./StdinImgCnt 
```

##Output
### Bash
```
Format identifier: BM
           0.282 ms (           0.468 ms) -- 3547.576127 fps (2136.752137 fps) -- 10000 frames
Input interupted! 3 sec timout reached!
```
### CSV
```
Frame, diff. Time, FPS
2, 0.552000,    1811.59420290
3, 0.525000,    1904.76190476
4, 0.497000,    2012.07243461
5, 0.521000,    1919.38579655
....
```

##Quirks
<li>First frame is lost to detect the format
<li>There are some spikes in the graphs. I don't exactly know why.
<li><i>I'm sure there are many more ...</i>
