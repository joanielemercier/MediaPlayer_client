# MediaPlayer_client
core player

Clients are identified by a unique string which is generated once and preserved between launches. The string can be edited in data/settings.xml.

## OSC Commands

Commands can be addressed to all clients or to a particular client.

To address a particular client using multicast, prefix the address with "/client/" and the unique identifier, eg

    /client/8fb1/frame_number

To address a particular output of a client using multicast, prefix the address with the prefix for the client, followed by /output/ and the unique identifier, eg

    /client/8fb1/output/surface3/warp/top_left/x

Numeric arguments can be represented in any form.

Coordinates have their origin bottom-left.

| address              | arguments                                           | scope                    |
|----------------------|-----------------------------------------------------|--------------------------|
| /add_output          | Name for the output (optional, default random)      | universe, client         |
| /delete_output       | Name of the output                                  | universe, client         |
| /full_screen         | 0 for windowed, any other value full-screen         | universe, client         |
| /display_stats       | 0 display off, any other value on                   | universe, client         |
| /source              | Source full or relative path                        | universe, client         |
| /frame_number        | Frame number                                        | universe, client         |
| /frame_number_reset  | A discontinuous frame number (optional, default 0)  | universe, client         |
| /send_config         | Server address and port (eg 192.168.0.10:2000)      | universe, client         |
| /crop/active         | 0 crop inactive, any other value active             | universe, client, output |
| /crop/x              | Horizontal crop origin in frame pixels              | universe, client, output |
| /crop/y              | Vertical crop origin in frame pixels                | universe, client, output |
| /crop/width          | Crop width in frame pixels                          | universe, client, output |
| /crop/height         | Crop height in frame pixels                         | universe, client, output |
| /warp/top_left/x     | Horizontal transform applied to top left corner     | universe, client, output |
| /warp/top_left/y     | Vertical transform applied to top left corner       | universe, client, output |
| /warp/top_right/x    | Horizontal transform applied to top right corner    | universe, client, output |
| /warp/top_right/y    | Vertical transform applied to top right corner      | universe, client, output |
| /warp/bottom_right/x | Horizontal transform applied to bottom right corner | universe, client, output |
| /warp/bottom_right/y | Vertical transform applied to bottom right corner   | universe, client, output |
| /warp/bottom_left/x  | Horizontal transform applied to bottom left corner  | universe, client, output |
| /warp/bottom_left/y  | Vertical transform applied to bottom left corner    | universe, client, output |
| /blend/left          | Blend distance in pixels from left edge             | universe, client, output |
| /blend/top           | Blend distance in pixels from top edge              | universe, client, output |
| /blend/right         | Blend distance in pixels from right edge            | universe, client, output |
| /blend/bottom        | Blend distance in pixels from bottom edge           | universe, client, output |

## The send_config Command

On receiving a `send_config` command, a client will return via OSC with address `/client/xml` to the provided host and port XML formatted:

    <config>
    	<id>90a1</id>
    	<extent>
    		<width>3840</width>
    		<height>1020</height>
    	</extent>
    	<source>Movie.mov</source>
    	<outputs>
    		<output id="1">
    			<name>1</name>
    		</output>
            <output id="2">
                <name>2</name>
            </output>
    	</outputs>
    </config> 

## XML Playlist Sources

An example XML source:

    <playlist>
        <sequence digits="3" suffix=".hpz" start="1" count="40">folder/image_name_prefix</sequence>
        <sequence digits="3" suffix=".hpz" start="400" count="2000">folder2/image_name_prefix</sequence>
        <file>folder/file_name_000.hpz</file>
        <file>folder/file_name_001.hpz</file>
    </playlist>

