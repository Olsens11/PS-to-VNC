#ifndef PSTVNC_AUDIO_PS2_RUNTIME_TEST_LOADFILE_H
#define PSTVNC_AUDIO_PS2_RUNTIME_TEST_LOADFILE_H
int SifLoadModule(const char *path, int argument_length, const char *arguments);
int SifExecModuleBuffer(void *module_buffer, unsigned int module_size,
    int argument_length, const char *arguments, int *result);
#endif
