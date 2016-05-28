/* OLIC implementation
 *
 * author: alei  mailto:rayingecho@hotmail.com
 */

#ifndef OLIC_HPP
#define OLIC_HPP

#include <stdlib.h>
#include <vector>
#include <glm/glm.hpp>
#include "vectorField.hpp"

struct OlicParam {
    // the forward or backward sample length in LIC, cooresponding to the notation 'L' in paper
    int sideLength = 50;

    // how many times allowed a streamline cross a pixel, 1 in OLIC
    int maxHitNum = 1;

    // how big the droplet is, equal to the radius of droplet or half streamline width approximately
    int dimPixel = 3;

    // imply how much droplet will be generate in the original texture, [0, 1]
    float dropletRate = 0.1;

    // integral step of Runge-Kutta methods, 0.5 pixel is recommended
    float integralStep = 0.5;

    // the source texture and ouput texure size
    int width = 1024;

    int height = 1024;
};

struct Droplet {
    int xPos = 0;
    int yPos = 0;
    int offset = 0;
    Droplet(int x, int y, int offset) {
        xPos = x;
        yPos = y;
        this->offset = offset;
    }
};

struct StreamLine {
    std::vector<glm::vec2> points;
    int length = 0;
};

/**
 * singleton, include Olic algorithm related datas and methods
 */
class OlicContext {
public:
    // static factory method that create or offer olicContext instance
    static OlicContext& init(OlicParam &olicParam, VectorField &field);
    
    
    // judge if the given point located in canvas
    bool isInclude(glm::vec2 point) const {
        return isInclude(std::pair<int, int>(round(point.x), round(point.y)));
    }
    bool isInclude(std::pair<int, int> point) const {
        return point.first >= 0 && point.first < _param->width && point.second >= 0 && point.second < _param->height;
    }

    // get the texel of the source texture in the given point
    float getSourceTexel(glm::vec2 point) const {
        return getSourceTexel(std::pair<int, int>(round(point.x), round(point.y)));
    }
    float getSourceTexel(std::pair<int, int> point) const {
        return _sourceTex[point.first + _param->width * point.second];
    }

    Droplet getRelateDroplet(glm::vec2 point) const {
        return getRelateDroplet(std::pair<int, int>(round(point.x), round(point.y)));
    }
    Droplet getRelateDroplet(std::pair<int, int> point) const {
        int index =  _relateDroplets[point.first + _param->width * point.second];
        return _droplets[index];
    }

    int getRelateDropletIndex(glm::vec2 point) const {
        return getRelateDropletIndex(std::pair<int, int>(round(point.x), round(point.y)));
    }
    int getRelateDropletIndex(std::pair<int, int> point) const {
        return _relateDroplets[point.first + _param->width * point.second];
    }

    int getHitCount(std::pair<int, int> point) const {
        return _hitCounts[point.first + _param->width * point.second];
    }

    /**
     * @brief refresh and return the OLIC texture every frame
     *
     * this method will check the cache for the certain phrase, if the cooresponding texture has not been calculated,
     * calcalate it and store it in the cache.
     */
    std::vector<glm::vec4> & refreshOLIC();

private:
    // the singleton instance
    static OlicContext* _instance;
    // olic algo parameters instance
    OlicParam* _param;
    // the low frequency texture map
    std::vector<float> _sourceTex;
    // the result texture
    std::vector<float> _resultTex;
    // count how many times a pixel is calculated
    std::vector<int> _hitCounts;
    // record all droplets
    std::vector<Droplet> _droplets;
    // record the index of responsibel droplet for each piexl
    std::vector<int> _relateDroplets;
    // cache for the cycle animation textures
    std::vector<std::vector<glm::vec4>> _texCache;
    // the vector field instance
    VectorField* _field;
    // global offset of ramp filter, change it to shift all the ramp filters.
    int _globalOffset;

    explicit OlicContext(OlicParam& olicParam, VectorField& field);

    void buildSourceTexture(OlicParam& olicParam);

    void calculateOLIC();

    StreamLine* calculateStreamLine(std::pair<int, int> point);

    void convolve(StreamLine* streamLine);

    float RampFilter(int pos, int localOffset, int sampleLength);
};

#endif
