//
//  Playlist.cpp
//  MediaPlayer_client
//
//  Created by Tom Butterworth on 14/03/2015.
//
//

#include "Playlist.h"

Playlist::Playlist(std::string path)
{
    load(path);
}

Playlist::Playlist()
{

}

Playlist::~Playlist()
{

}

void Playlist::load(std::string path)
{
    paths_.clear();
    base_ = ofFilePath::getEnclosingDirectory(path);
    ofXml xml(path);
    if (xml.setTo("playlist"))
    {
        if (xml.setToChild(0))
        {
            do {
                std::string element_name = xml.getName();
                if (element_name == "sequence")
                {
                    std::string sequence_base = xml.getValue();
                    int digits = ofFromString<int>(xml.getAttribute("digits"));
                    std::string suffix = xml.getAttribute("suffix");
                    unsigned int count = ofFromString<unsigned int>(xml.getAttribute("count"));
                    unsigned int start = ofFromString<unsigned int>(xml.getAttribute("start"));
                    for (unsigned int i = 0; i < count; i++) {
                        paths_.push_back(sequence_base + ofToString(start + i, digits, '0') + suffix);
                    }
                }
                else if (element_name == "file")
                {
                    paths_.push_back(xml.getValue());
                }
            } while (xml.setToSibling());
        }

    }
}

unsigned int Playlist::size()
{
    return paths_.size();
}

std::string Playlist::operator[](unsigned int index) const
{
    return paths_[index];
}
