//
//  This file is part of the ofxDepthStream [https://github.com/fusefactory/ofxDepthStream]
//  Copyright (C) 2018 Fuse srl
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//

/**
 * \file This file provides a functional-programming interface to this addon's OpenFrameworks-specific API
 */

#include "functional.h"

/**
 * populates the texture instance with 16-bit grayscale depth-image data from the given frame-data. It allocates the texture if necessary.
 */
void ofxDepthStream::loadDepthTexture8bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts) {
  // allocate
  if(!tex.isAllocated()) {
    // ofLogNotice() << "Allocating depth texture";
    if(size == FRAME_SIZE_640x240x08BIT) {
      tex.allocate(640, 240, GL_RGB);
    } else {
      ofLogWarning() << "Frame-size not supported by ofxDepthStream::loadDepthTexture8bit: " << size;
      return;
    }
  }

  // // check
  // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
  // if(expectedSize != size) {
  //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
  //   return;
  // }

  // 3-channel data buffer
  float* raw = new float[(int)tex.getWidth() * (int)tex.getHeight() * 3];

  for (int y = 0.0; y < tex.getHeight(); y++) {
    for (int x = 0.0; x < tex.getWidth(); x++) {
      // keystone
      int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * opts.keystone * (x - tex.getWidth() / 2.0);

      // convert multi-byte into single depth value
      int srcIndex = int((posX + y * tex.getWidth()));
      // unsigned int byte0 = ((unsigned char*)data)[srcIndex + 0];
      // unsigned int byte1 = ((unsigned char*)data)[srcIndex + 1];
      // unsigned int depth = ((byte0 & 0xFF) << (8 + opts.shift1)) | ((byte1 & 0xFF) << opts.shift2);
      unsigned char depth = ((unsigned char*)data)[srcIndex];
      int dstIndex = (x + y * tex.getWidth());
      bool valid = false;

      if (
        // top margin
        y >= opts.margins[0]
        // left margin
        && posX >= opts.margins[3]
        // right margin
        && posX <= tex.getWidth() - opts.margins[1]
        // bottom margin
        && y <= tex.getHeight() - opts.margins[2])
      {
        int correctMaxDistance = std::min(opts.maxDistance, 255) * (1.0 - opts.vertCorrection * (std::cos(glm::pi<float>() / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
        if (depth >= opts.minDistance && depth <= correctMaxDistance) {
          float intensity = (depth - opts.minDistance) / (float)(correctMaxDistance - opts.minDistance);
          raw[dstIndex * 3 + 0] = 1 - intensity;
          raw[dstIndex * 3 + 1] = 1 - intensity;
          raw[dstIndex * 3 + 2] = 1 - intensity;
          valid = true;
        }
      }
      if (!valid) {
        raw[dstIndex * 3 + 0] = 0.0;
        raw[dstIndex * 3 + 1] = 0.0;
        raw[dstIndex * 3 + 2] = 0.0;
      }
    }
  }

  tex.loadData((float*)raw, tex.getWidth(), tex.getHeight(), GL_RGB);
  // ofLogNotice() << "loaded edge data texture";//: maxFoundDist: " << maxFoundDist;
}

/**
 * populates the texture instance with 16-bit grayscale depth-image data from the given frame-data. It allocates the texture if necessary.
 */
void ofxDepthStream::loadDepthTexture16bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts) {
  // allocate
  if(!tex.isAllocated()) {
    // ofLogNotice() << "Allocating depth texture";
    if(size == FRAME_SIZE_640x480x16BIT) {
      tex.allocate(640, 480, GL_RGB);
    } else if (size == FRAME_SIZE_1280x720x16BIT) {
      tex.allocate(1280, 720, GL_RGB);
    } else if(size == FRAME_SIZE_512x424x16BIT){
      tex.allocate(512, 424, GL_RGB);
    } else {
      ofLogWarning() << "Frame-size not supported by ofxDepthStream::loadDepthTexture16bit: " << size;
      return;
    }
  }

  // // check
  // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
  // if(expectedSize != size) {
  //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
  //   return;
  // }

  // 3-channel data buffer
  float* raw = new float[(int)tex.getWidth() * (int)tex.getHeight() * 3];

  for (int y = 0.0; y < tex.getHeight(); y++) {
    for (int x = 0.0; x < tex.getWidth(); x++) {
      // keystone
      int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * opts.keystone * (x - tex.getWidth() / 2.0);

      // convert multi-byte into single depth value
      int srcIndex = int((posX + y * tex.getWidth()) * 2);
      // unsigned int byte0 = ((unsigned char*)data)[srcIndex + 0];
      // unsigned int byte1 = ((unsigned char*)data)[srcIndex + 1];
      // unsigned int depth = ((byte0 & 0xFF) << (8 + opts.shift1)) | ((byte1 & 0xFF) << opts.shift2);
      uint16_t depth = *((uint16_t*)(&((char*)data)[srcIndex]));
      int dstIndex = (x + y * tex.getWidth());
      bool valid = false;

      if (
        // top margin
        y >= opts.margins[0]
        // left margin
        && posX >= opts.margins[3]
        // right margin
        && posX <= tex.getWidth() - opts.margins[1]
        // bottom margin
        && y <= tex.getHeight() - opts.margins[2])
      {
        int correctMaxDistance = opts.maxDistance * (1.0 - opts.vertCorrection * (std::cos(glm::pi<float>() / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
        if (depth >= opts.minDistance && depth <= correctMaxDistance) {
          float intensity = (depth - opts.minDistance) / (float)(correctMaxDistance - opts.minDistance);
          raw[dstIndex * 3 + 0] = 1 - intensity;
          raw[dstIndex * 3 + 1] = 1 - intensity;
          raw[dstIndex * 3 + 2] = 1 - intensity;
          valid = true;
        }
      }
      if (!valid) {
        raw[dstIndex * 3 + 0] = 0.0;
        raw[dstIndex * 3 + 1] = 0.0;
        raw[dstIndex * 3 + 2] = 0.0;
      }
    }
  }

  tex.loadData((float*)raw, tex.getWidth(), tex.getHeight(), GL_RGB);
  // ofLogNotice() << "loaded edge data texture";//: maxFoundDist: " << maxFoundDist;
}

/**
 * populates the texture instance with 32-bit grayscale depth-image data from the given frame-data. It allocates the texture if necessary.
 */
void ofxDepthStream::loadDepthTexture32bit(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts) {
  // allocate
  if(!tex.isAllocated()) {
    // ofLogNotice() << "Allocating depth texture";
    if(size == FRAME_SIZE_640x480x32BIT) {
      tex.allocate(640, 480, GL_RGB);
    } else if(size == FRAME_SIZE_512x424x32BIT){
      tex.allocate(512, 424, GL_RGB);
    } else {
      ofLogWarning() << "Frame-size not supported by ofxDepthStream::loadDepthTexture32bit: " << size;
      return;
    }
  }

  // // check
  // size_t expectedSize = tex.getWidth() * tex.getHeight() * 2;
  // if(expectedSize != size) {
  //   ofLogWarning() << "Edge-data texture size did not match data-size (got: " << size << ", expected: " << expectedSize << ")";
  //   return;
  // }

  // 3-channel data buffer
  float* raw = new float[(int)tex.getWidth() * (int)tex.getHeight() * 3];

  for (int y = 0.0; y < tex.getHeight(); y++) {
    for (int x = 0.0; x < tex.getWidth(); x++) {
      // keystone
      int posX = x + ((y - tex.getHeight() / 2.0) / (tex.getHeight() / 2.0)) * opts.keystone * (x - tex.getWidth() / 2.0);

      // convert multi-byte into single depth value
      int srcIndex = int((posX + y * tex.getWidth()) * 4);
      int byte0 = ((unsigned char*)data)[srcIndex + 0];
      int byte1 = ((unsigned char*)data)[srcIndex + 1];
      int byte2 = ((unsigned char*)data)[srcIndex + 2];
      int byte3 = ((unsigned char*)data)[srcIndex + 3];
      int depth = byte0 << 24 | (byte1 & 0xFF) << 16 | (byte2 & 0xFF) << 8 | (byte3 & 0xFF);

      int dstIndex = (x + y * tex.getWidth());
      bool valid = false;

      if (
        // top margin
        y >= opts.margins[0]
        // left margin
        && posX >= opts.margins[3]
        // right margin
        && posX <= tex.getWidth() - opts.margins[1]
        // bottom margin
        && y <= tex.getHeight() - opts.margins[2])
      {
        int correctMaxDistance = opts.maxDistance * (1.0 - opts.vertCorrection * (std::cos(glm::pi<float>() / 3.0 * (tex.getHeight() - y) / tex.getHeight()) - 0.5));
        if (depth >= opts.minDistance && depth <= correctMaxDistance) {
          float intensity = (depth - opts.minDistance) / (float)(correctMaxDistance - opts.minDistance);
          raw[dstIndex * 3 + 0] = 1 - intensity;
          raw[dstIndex * 3 + 1] = 1 - intensity;
          raw[dstIndex * 3 + 2] = 1 - intensity;
          valid = true;
        }
      }
      if (!valid) {
        raw[dstIndex * 3 + 0] = 0.0;
        raw[dstIndex * 3 + 1] = 0.0;
        raw[dstIndex * 3 + 2] = 0.0;
      }
    }
  }

  tex.loadData((float*)raw, tex.getWidth(), tex.getHeight(), GL_RGB);
  // ofLogNotice() << "loaded edge data texture";//: maxFoundDist: " << maxFoundDist;
}

/**
 * populates the texture instance with grayscale depth-image data from the given frame-data. It allocates the texture if necessary and guesses the bit-depth of the data based on the size of the package.
 */
void ofxDepthStream::loadDepthTexture(ofTexture& tex, const void* data, size_t size, const DepthLoaderOpts& opts) {
  if (size == FRAME_SIZE_640x480x16BIT || size == FRAME_SIZE_512x424x16BIT) {
    loadDepthTexture16bit(tex, data, size, opts);
    return;
  }

  if (size == FRAME_SIZE_640x480x32BIT || size == FRAME_SIZE_512x424x32BIT) {
    loadDepthTexture32bit(tex, data, size, opts);
    return;
  }

  if (size == FRAME_SIZE_640x240x08BIT) {
    loadDepthTexture8bit(tex, data, size, opts);
    return;
  }

  if (size == FRAME_SIZE_1280x720x16BIT) {
    loadDepthTexture16bit(tex, data, size, opts);
    return;
  }

  ofLogWarning() << "Frame size not supported by ofxDepthStream::loadDepthTexture (bytes): " << size;
}

/**
 * Populates texture with depth data from the buffer's current content and empties the buffer.
 *
 * The texture is allocated if necessary, the resolution and bit-depth is guessed based on the size of the buffer's content.
 */
void ofxDepthStream::loadDepthTexture(depth::Buffer& buffer, ofTexture& tex, const DepthLoaderOpts& opts ) {
  // check if buffer has data
  depth::emptyAndInflateBuffer(buffer, [&tex, &opts](const void* data, size_t size){
    loadDepthTexture(tex, data, size, opts);
  }, opts);
}


// Mesh Loader methods // // // // //


/**
 * populates the mesh instance with 16-bit depth-image data. It allocates the texture if necessary.
 */
void ofxDepthStream::loadMesh16bit(ofMesh& mesh, const void* data, size_t width, size_t height, const MeshLoaderOpts& opts) {
  const uint16_t* pointData = (const uint16_t*)data;

  mesh.clear();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint16_t val = pointData[y*width+x];
      ofVec3f p(x,y, val * opts.depthFactor);
      mesh.addVertex(p);

      // TODO; provide some coloring options
      float hue  = ofMap(val, 0, 6000, 0, 255);
      mesh.addColor(ofColor::fromHsb(hue, 255, 255));
    }
  }
}

/**
 * UNTESTED populates the mesh instance with 32-bit depth-image data. It allocates the texture if necessary.
 */
void ofxDepthStream::loadMesh32bit(ofMesh& mesh, const void* data, size_t width, size_t height, const MeshLoaderOpts& opts) {
  const uint32_t* pointData = (const uint32_t*)data;

  mesh.clear();

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      uint32_t val = pointData[y*width+x]; // TODO/TOTEST; do manual byte processing here?
      ofVec3f p(x,y, val * opts.depthFactor);
      mesh.addVertex(p);

      // TODO; provide some coloring options
      float hue  = ofMap(val, 0, 6000, 0, 255);
      mesh.addColor(ofColor::fromHsb(hue, 255, 255));
    }
  }
}

/**
 * populates the mesh instance with depth-image data.
 *
 * It allocates the texture if necessary, and guesses the dimensions and bit-depth of the package size
 */
void ofxDepthStream::loadMesh(ofMesh& mesh, const void* data, size_t size, const MeshLoaderOpts& opts) {
  if(size == FRAME_SIZE_640x480x16BIT) {
    loadMesh16bit(mesh, data, 640, 480, opts);
    return;
  }

  if(size == FRAME_SIZE_640x480x32BIT) {
    loadMesh32bit(mesh, data, 640, 480, opts);
    return;
  }

  if(size == FRAME_SIZE_512x424x16BIT){
    loadMesh16bit(mesh, data, 512, 424, opts);
    return;
  }

  if(size == FRAME_SIZE_512x424x32BIT){
    loadMesh32bit(mesh, data, 512, 424, opts);
    return;
  }

  if(size == FRAME_SIZE_1280x720x16BIT){
    loadMesh16bit(mesh, data, 1280, 720, opts);
    return;
  }

  ofLogWarning() << "Frame size not supported by ofxDepthStream::loadMesh (bytes): " << size;
}

/**
 * populates the mesh instance with depth-image data taken from the buffer's current content (if any).
 *
 * It allocates the texture if necessary, and guesses the dimensions and bit-depth of the package size.
 * It empties the buffer after using its content.
 */
void ofxDepthStream::loadMesh(depth::Buffer& buffer, ofMesh& mesh, const MeshLoaderOpts& opts) {
  // check if buffer has data
  depth::emptyAndInflateBuffer(buffer, [&mesh, &opts](const void* data, size_t size){
    loadMesh(mesh, data, size, opts);
  });
}


// Color texture loader methods (untested) // // // // //

/**
 * UNTESTED populates the texture with color-image data from the given frame-data. It allocates the texture if necessary.
 */
void ofxDepthStream::loadColorTexture(ofTexture& tex, const void* data, size_t size) {
  ofPixels pixels;

  // allocate
  if(tex.isAllocated()) {
    pixels.allocate(tex.getWidth(), tex.getHeight(), OF_IMAGE_COLOR);
  } else {
    ofLogWarning() << "TODO: infer color texture resolution from data-size";
    pixels.allocate(1280, 720, OF_IMAGE_COLOR);
    tex.allocate(pixels);
  }

  // check
  if((tex.getWidth() * tex.getHeight()) * 3 != size) {
    ofLogWarning() << "Color texture size did not match data-size (got: " << size << ", expected: 1280x720x3=2764800)";
    return;
  }

  // load
  pixels.setFromPixels((const unsigned char *)data, pixels.getWidth(), pixels.getHeight(), OF_IMAGE_COLOR);
  tex.loadData(pixels);
}

/**
 * UNTESTED populates the texture with color-image data taken from the buffer's current content (if any).
 *
 * It allocates the texture if necessary, and guesses the dimensions and bit-depth of the package size.
 * It empties the buffer after using its content.
 */
void ofxDepthStream::loadColorTexture(depth::Buffer& buffer, ofTexture& tex) {
  // check if buffer has data
  depth::emptyAndInflateBuffer(buffer, [&tex](const void* data, size_t size){
    loadColorTexture(tex, data, size);
  });
}
