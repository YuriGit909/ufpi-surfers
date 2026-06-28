#define MINIAUDIO_IMPLEMENTATION
#include "external/miniaudio.h"
#include <iostream>
#include <fstream>

static ma_engine engine;
static ma_sound music;
static bool initialized = false;

void initAudio()
{
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS)
    {
        std::cout << "Erro ao iniciar o audio." << std::endl;
        return;
    }

    initialized = true;
}

void playMusic()
{
    if (!initialized)
        return;


std::ifstream f("./assets/sounds/upfisurfers.mp3");

if (!f.good())
{
    std::cout << "Arquivo nao encontrado!" << std::endl;
}
else
{
    std::cout << "Arquivo encontrado!" << std::endl;
}

    if (ma_sound_init_from_file(
            &engine,
            "./assets/sounds/ufpisurfers.mp3",
            MA_SOUND_FLAG_STREAM,
            NULL,
            NULL,
            &music) != MA_SUCCESS)
    {
        std::cout << "Erro ao carregar musica." << std::endl;
        return;
    }

    ma_sound_set_looping(&music, MA_TRUE);
    ma_sound_start(&music);
}

void stopAudio()
{
    if (!initialized)
        return;

    ma_sound_uninit(&music);
    ma_engine_uninit(&engine);
}