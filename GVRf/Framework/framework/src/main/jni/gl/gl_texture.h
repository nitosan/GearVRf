/* Copyright 2015 Samsung Electronics Co., LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/***************************************************************************
 * RAII class for GL textures.
 ***************************************************************************/

#ifndef GL_TEXTURE_H_
#define GL_TEXTURE_H_

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif /* GL_EXT_texture_filter_anisotropic */

#include "gl/gl_headers.h"

#include "util/gvr_log.h"
#include <cstdlib>
#include <cstring>

#include "objects/gl_pending_task.h"

#define MAX_TEXTURE_PARAM_NUM 10

namespace gvr {
class GLTexture : public GLPendingTask {
public:
    explicit GLTexture(GLenum target)
    : target_(target)
    , pending_gl_task_(GL_TASK_NONE)
    {
        pending_gl_task_ = GL_TASK_INIT_NO_PARAM;
    }
    explicit GLTexture(GLenum target, int texture_id) :
            target_(target) {
        id_ = texture_id;
    }
    explicit GLTexture(GLenum target, int* texture_parameters) :
            target_(target) {
        pending_gl_task_ = GL_TASK_INIT_WITH_PARAM;
        std::memcpy(texture_parameters_, texture_parameters, sizeof(texture_parameters_));
    }

    virtual ~GLTexture() {
        if (0 != id_) {
            GL(glDeleteTextures(1, &id_));
        }
    }

    GLuint id() {
        runPendingGL();
        return id_;
    }

    GLenum target() const {
        return target_;
    }

    virtual void runPendingGL() {
        switch (pending_gl_task_) {
        case GL_TASK_NONE:
            return;

        case GL_TASK_INIT_NO_PARAM: {
            glGenTextures(1, &id_);
            glBindTexture(target_, id_);
            glTexParameteri(target_, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(target_, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(target_, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(target_, 0);
            break;
        }

        case GL_TASK_INIT_WITH_PARAM: {
            // Sets the new MIN FILTER
            GLenum min_filter_type_ = texture_parameters_[0];

            // Sets the MAG FILTER
            GLenum mag_filter_type_ = texture_parameters_[1];

            // Sets the wrap parameter for texture coordinate S
            GLenum wrap_s_type_ = texture_parameters_[3];

            // Sets the wrap parameter for texture coordinate S
            GLenum wrap_t_type_ = texture_parameters_[4];

            glGenTextures(1, &id_);
            glBindTexture(target_, id_);

            // Sets the anisotropic filtering if the value provided is greater than 1 because 1 is the default value
            if (texture_parameters_[2] > 1.0f) {
                glTexParameterf(target_, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                        (float) texture_parameters_[2]);
            }

            glTexParameteri(target_, GL_TEXTURE_WRAP_S, wrap_s_type_);
            glTexParameteri(target_, GL_TEXTURE_WRAP_T, wrap_t_type_);
            glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, min_filter_type_);
            glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, mag_filter_type_);

            const GLint internalFormat = texture_parameters_[5];
            const GLint width = texture_parameters_[6];
            const GLint height = texture_parameters_[7];
            const GLint format = texture_parameters_[8];
            const GLint type = texture_parameters_[9];
            if (0 < internalFormat && 0 < width && 0 < height && 0 < format && 0 < type) {
                glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
            }

            glBindTexture(target_, 0);
            break;
        }

        } // switch

        pending_gl_task_ = GL_TASK_NONE;
    }

private:
    GLTexture(const GLTexture& gl_texture);
    GLTexture(GLTexture&& gl_texture);
    GLTexture& operator=(const GLTexture& gl_texture);
    GLTexture& operator=(GLTexture&& gl_texture);

private:
    GLuint id_ = 0;
    GLenum target_;

    // Enum for pending GL tasks. Keep a comma with each line
    // for easier merging.
    enum {
        GL_TASK_NONE = 0,
        GL_TASK_INIT_NO_PARAM,
        GL_TASK_INIT_WITH_PARAM,
    };
    int pending_gl_task_;

    int texture_parameters_[MAX_TEXTURE_PARAM_NUM];
};

}
#endif
