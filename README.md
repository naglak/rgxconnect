<<<<<<< HEAD
=======

>>>>>>> 526cff8 (initial commit)
# RGXCONNECT

rgxconnect is an application which connects newly registered jack clients ports
according to regex rules.

## Usage

```
rgxconnect  [-s servername] [-j clientname] config_file
```
## Config file syntax
One rule in each line.
```
<new_registered_port> <existing_port>
```
## Examples
Firefox uses its PID int port name.
```
<Firefox:AudioStream.*out_0> <zita-j2n:in_1>                                                              
<Firefox:AudioStream.*out_1> <zita-j2n:in_2>
```
Connects up to 32 ports.
Even ports to ```system:playback_1``` and odd ports to ```system:playback_2```
```
<zita-a2j:capture_[1-9]*[13579]$><system:playback_1>
<zita-a2j:capture_[1-9]*[02468]$><system:playback_2>
```
Vlc uses its PID in port name.
```
<vlc_.*:.*out_1><system:playback_1>
<vlc_.*:.*out_2><system:playback_2> 
```
Clients with "<" or ">" characters in names.
```
<example\<name><system:playback_1>
```
## License


Copyright (C) 2023, 2024 Jacek Naglak

This program is free software;you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
