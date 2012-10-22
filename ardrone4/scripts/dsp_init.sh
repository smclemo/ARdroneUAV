#!/bin/sh


/bin/dspbridge/cexec.out -T /system/lib/dsp/baseimage.dof
/bin/dspbridge/dynreg.out -r /system/lib/dsp/720p_h264venc_sn.dll64P
/bin/dspbridge/dynreg.out -r /system/lib/dsp/usn.dll64P

