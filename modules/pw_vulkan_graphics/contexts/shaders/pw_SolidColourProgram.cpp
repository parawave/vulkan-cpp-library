/*
  ==============================================================================

   This file is part of the Parawave Vulkan C++ library.

   The code included in this file is provided under the terms of the ISC license
   https://opensource.org/licenses/ISC.

   Copyright (c) 2021 - Parawave Audio (https://parawave-audio.com/vulkan-cpp-library)

   Permission to use, copy, modify, and/or distribute this software for any 
   purpose with or without fee is hereby granted, provided that the above 
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF 
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
   SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
   OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

namespace parawave
{
    
//==============================================================================
class SolidColourProgram
{
private:
    struct PipelineInfo : public ProgramHelpers::GraphicsPipelineCreateInfo
    {
        PipelineInfo(VulkanDevice& device, const VulkanPipelineLayout& pipelineLayout, const VulkanRenderPass& renderPass)
            : ProgramHelpers::GraphicsPipelineCreateInfo(pipelineLayout, renderPass)
        {
            setShaders(device, "SolidColour.vert", "SolidColour.frag");

            finish();
        }
    };

public:
    SolidColourProgram(VulkanDevice& device, const VulkanRenderPass& renderPass) :
        pipelineLayout(device, ProgramHelpers::PipelineLayoutInfo()),
        pipeline(device, PipelineInfo(device, pipelineLayout, renderPass)) { }

    const VulkanPipelineLayout pipelineLayout;
    const VulkanPipeline pipeline;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolidColourProgram)
};

//==============================================================================
/*
    layout (local_size_x = 256) in;

    layout(set = 0, binding = 0) uniform Config{   
        mat4 transform;      
        int matrixCount;
    } opData;

    layout(set = 0, binding = 1) readonly buffer  InputBuffer{   
        mat4 matrices[];   
    } sourceData;

    layout(set = 0, binding = 2) buffer  OutputBuffer{   
        mat4 matrices[];   
    } outputData;


    void main() 
    {		
        //grab global ID
	    uint gID = gl_GlobalInvocationID.x;
        //make sure we don't access past the buffer size
        if(gID < matrixCount)
        {
            // do math
            outputData.matrices[gID] = sourceData.matrices[gID] * opData.transform;
        }
    }

    int groupcount = ((num_matrices) / 256) + 1;
*/


/*
    layout (local_size_y = 256) in;

    layout(push_constant) uniform PushConsts {
	    vec4 bounds;
        int numLines;
    } pc;

    struct LineItem
    {
      int x;
      int level;
    };

    struct Scanline
    {
        int numPoints;
        LineItem table[];
    };

    layout(set = 0, binding = 0) readonly buffer InputBuffer {   
        Scanline scanlines[];
    } sourceData;

    layout(set = 0, binding = 1) buffer OutputBuffer {   
        mat4 matrices[];   
    } outputData;

    void main() 
    {		
	    uint gID = gl_GlobalInvocationID.y;
        if(gID < numLines)
        {
            int numPoints = sourceData.scanlines[gId].numPoints;
            if (--numPoints > 0)
            {
                int i = 0;

                int x = sourceData.scanlines[gId].table[i];
                int levelAccumulator = 0;

                int py = pc.bounds.y + gl_GlobalInvocationID.y;

                while (--numPoints >= 0)
                {
                    const int level = sourceData.scanlines[gId].table[i];
                    ++i;

                    const int endX = sourceData.scanlines[gId].table[i];
                    ++i;

                    int endOfRun = (endX >> 8);
                    if (endOfRun == (x >> 8))
                    {
                        // small segment within the same pixel, so just save it for the next
                        // time round..
                        levelAccumulator += (endX - x) * level;
                    }
                    else
                    {
                        // plot the fist pixel of this segment, including any accumulated
                        // levels from smaller segments that haven't been drawn yet
                        levelAccumulator += (0x100 - (x & 0xff)) * level;
                        levelAccumulator >>= 8;
                        x >>= 8;

                        if (levelAccumulator > 0)
                        {
                            if (levelAccumulator >= 255)
                            {
                                //iterationCallback.handleEdgeTablePixelFull (x);
                            }
                            else
                            {
                                //iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                            }
                        }

                        // if there's a run of similar pixels, do it all in one go..
                        if (level > 0)
                        {
                            int numPix = endOfRun - ++x;

                            if (numPix > 0)
                            {
                                //iterationCallback.handleEdgeTableLine (x, numPix, level);
                            }  
                        }

                        // save the bit at the end to be drawn next time round the loop.
                        levelAccumulator = (endX & 0xff) * level;
                    }

                    x = endX;
                }

                levelAccumulator >>= 8;
                
                if (levelAccumulator > 0)
                {
                    x >>= 8;

                    if (levelAccumulator >= 255)
                    {
                        // iterationCallback.handleEdgeTablePixelFull (x);
                    }
                    else
                    {
                        // iterationCallback.handleEdgeTablePixel (x, levelAccumulator);
                    }
                }
            }
        }
    }
*/

class EdgeTableComputeProgram
{
private:



};

} // namespace parawave