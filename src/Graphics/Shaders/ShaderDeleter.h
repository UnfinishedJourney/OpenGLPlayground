#pragma once

#include <glad/glad.h>
#include "Utilities/Logger.h"

struct ShaderDeleter {
    void operator()(GLuint* programID) const {
        if (programID && *programID != 0) {
            glDeleteProgram(*programID);
            Logger::GetLogger()->info("Deleted shader program with ID: {}", *programID);
            delete programID;
        }
    }
};