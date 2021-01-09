//
// Created by asalehin on 9/9/20.
//

#ifndef FAST_MIXER_MIXINGIO_H
#define FAST_MIXER_MIXINGIO_H


#include <memory>
#include <FileDataSource.h>
#include "conf/StreamConstants.h"
#include "../audio/Player.h"

using namespace std;

class MixingIO {
public:
    shared_ptr<FileDataSource> readFile(string filename);

private:
    shared_ptr<Player> mPlayer {nullptr};
};


#endif //FAST_MIXER_MIXINGIO_H
