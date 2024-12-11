#pragma once

#include <glad/glad.h>
#include "Utilities/Utility.h"
#include <cstdint>

class GPUQuery
{
public:
    // Constructor initializes the query with the specified target (e.g., GL_TIME_ELAPSED)
    GPUQuery(GLenum target)
        : m_target(target)
    {
        GLCall(glGenQueries(1, &m_queryID));
    }

    // Destructor cleans up the query object
    ~GPUQuery()
    {
        GLCall(glDeleteQueries(1, &m_queryID));
    }

    // Begins the query
    void Begin() const
    {
        GLCall(glBeginQuery(m_target, m_queryID));
    }

    // Ends the query
    void End() const
    {
        GLCall(glEndQuery(m_target));
    }

    // Retrieves the result of the query if available
    bool GetResult(uint64_t& result) const
    {
        GLint available = 0;
        GLCall(glGetQueryObjectiv(m_queryID, GL_QUERY_RESULT_AVAILABLE, &available));
        if (available)
        {
            GLCall(glGetQueryObjectui64v(m_queryID, GL_QUERY_RESULT, &result));
            return true;
        }
        return false;
    }

private:
    GLuint m_queryID = 0;
    GLenum m_target;
};