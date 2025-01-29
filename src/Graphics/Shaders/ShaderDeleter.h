#pragma once

#include <glad/glad.h>
#include "Utilities/Logger.h"

/**
 * @brief Custom deleter for an OpenGL shader program.
 *        Used by std::unique_ptr<GLuint, ShaderDeleter>.
 */
struct ShaderDeleter {
    void operator()(GLuint* programID) const {
        if (programID && *programID != 0) {
            glDeleteProgram(*programID);
            Logger::GetLogger()->info("Deleted shader program with ID: {}", *programID);
        }
        delete programID;
    }
};