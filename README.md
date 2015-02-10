# MediaPlayer_client
core player

Clients are identified by a unique string which is generated once and preserved between launches. The string can be edited in data/settings.xml.

## OSC Commands

Commands can be addressed to all clients or to a particular client.

To address a particular client using multicast, prefix the address with "/client/" and the unique identifier, eg

    /client/8fb1/frame_number

Numeric arguments can be represented in any form.

Coordinates have their origin bottom-left.


| address              | arguments                                           |
|----------------------|-----------------------------------------------------|
| /frame_number        | Frame number                                        |
| /frame_number_reset  | A discontinuous frame number (optional, default 0)  |
| /display_stats       | 0 display off, any other value on                   |
| /crop/active         | 0 crop inactive, any other value active             |
| /crop/x              | Horizontal crop origin in frame pixels              |
| /crop/y              | Vertical crop origin in frame pixels                |
| /crop/width          | Crop width in frame pixels                          |
| /crop/height         | Crop height in frame pixels                         |
| /warp/top_left/x     | Horizontal transform applied to top left corner     |
| /warp/top_left/y     | Vertical transform applied to top left corner       |
| /warp/top_right/x    | Horizontal transform applied to top right corner    |
| /warp/top_right/y    | Vertical transform applied to top right corner      |
| /warp/bottom_right/x | Horizontal transform applied to bottom right corner |
| /warp/bottom_right/y | Vertical transform applied to bottom right corner   |
| /warp/bottom_left/x  | Horizontal transform applied to bottom left corner  |
| /warp/bottom_left/y  | Vertical transform applied to bottom left corner    |
| /blend/left          | Blend distance in pixels from left edge             |
| /blend/top           | Blend distance in pixels from top edge              |
| /blend/right         | Blend distance in pixels from right edge            |
| /blend/bottom        | Blend distance in pixels from bottom edge           |
| /full_screen         | 0 for windowed, any other value full-screen         |
