//
//  Playlist.h
//  MediaPlayer_client
//
//  Created by Tom Butterworth on 14/03/2015.
//
//

#ifndef __MediaPlayer_client__Playlist__
#define __MediaPlayer_client__Playlist__

#include "ofMain.h"

class Playlist {
public:
    Playlist();
    Playlist(std::string path);
    ~Playlist();
    void load(std::string path);
    unsigned int size();
    std::string operator [](unsigned int index) const;
private:
    std::vector<std::string> paths_;
    std::string base_;
};

#endif /* defined(__MediaPlayer_client__Playlist__) */
