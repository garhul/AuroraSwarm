


# format for messages TBD

Max message size 64 bytes ?

CMD:ARG1:ARG2:ARG3 ... etc
hex args?
hex cmd ? 


- FX:0:100:FF
- FX:1:40:FF
- RN:1:20:255:0:0 (range 1 to 20, color 255:0:0)
- FS:255:255:255 (fill strip)
- BR:255 (set brigthness)
- SP:100 (set animation speed)

- S:BR:100 (set br 100)
- S:SP:100 (set sp 100)
- G:SP (get sp)
- G:BR (get br)

- RS (reset)
- UP (update - OTA)


F:IDX:SPD:XXXX (FX IDX SPD XXXtra params)
P:1 | P:0 (play /pause)
O (off)