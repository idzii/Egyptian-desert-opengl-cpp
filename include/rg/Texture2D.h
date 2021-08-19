//
// Created by matf-rg on 30.10.20..
//

#ifndef PROJECT_BASE_TEXTURE2D_H
#define PROJECT_BASE_TEXTURE2D_H
#include <glad/glad.h>
#include <stb_image.h>
#include <rg/Error.h>

class Texture2D {
public:
    unsigned m_tex;
    unsigned char * m_data;

    Texture2D(GLenum wrap_s, GLenum wrap_t, GLenum mag_filter, GLenum min_filter){
        glGenTextures(1,&m_tex);
        glBindTexture(GL_TEXTURE_2D, m_tex);

        //wrap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);

        //filter
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
    }

    void reflect_vertically(){
        stbi_set_flip_vertically_on_load(true);
    }

    void load(std::string path_to_img, bool gamma_correction){
        int width, height, n_channels;

        m_data = stbi_load(path_to_img.c_str(), &width, &height, &n_channels, 0);

        GLenum internalFormat, dataFormat;
        if(n_channels == 1){
            internalFormat = dataFormat = GL_RED;
        }

        else if(n_channels == 3){
            internalFormat = gamma_correction ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }

        else if(n_channels == 4){
            internalFormat = gamma_correction ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        if(m_data){
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, m_data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else{
            std::cerr << "Failed to load texture\n";
        }
    }

    void free_data(){
        stbi_image_free(m_data);
    }

    void activate(GLenum texture_number) {
        glActiveTexture(texture_number);
        glBindTexture(GL_TEXTURE_2D, m_tex);
    }
};


#endif //PROJECT_BASE_TEXTURE2D_H
