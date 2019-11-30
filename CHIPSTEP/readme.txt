convert supercc route to stepmania

usage: chipstep > out.txt

expects udrl directions from supercc route (NOT the entire json) as
standard input (or chipstep < in.txt > out.txt if input is in a text file)

the output is not a complete stepfile. use an existing .sm file as a template
and copy the contents of out.txt into that.

you'll also need a song that's exactly 136.36 bpm.