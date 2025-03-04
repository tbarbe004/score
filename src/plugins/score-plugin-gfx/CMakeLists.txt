project(score_plugin_gfx LANGUAGES C CXX)

find_package(${QT_VERSION} OPTIONAL_COMPONENTS ShaderTools)
if(NOT TARGET ${QT_PREFIX}::ShaderTools)
  message("Video plug-in requires Qt Shader Tools (5.14+)")
  return()
endif()

# General initialization
score_common_setup()

find_package(${QT_VERSION} REQUIRED Gui)

if(NOT TARGET avformat)
  find_package(FFmpeg COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE)
endif()

add_subdirectory(3rdparty/libisf)

#if(WIN32)
#set(SPOUT_SRCS
#    Gfx/SpoutDevice.hpp
#    Gfx/SpoutDevice.cpp
#)
#endif()

find_package(freenect2)
if(NOT "${freenect2_LIBRARIES}")
  set(freenect2_FOUND FALSE)
endif()
if(freenect2_FOUND)
  set(KINECT2_SRCS Gfx/Kinect2Device.hpp Gfx/Kinect2Device.cpp)
endif()
# Source files
set(HDRS
    Gfx/CommandFactory.hpp

    Gfx/Filter/Executor.hpp
    Gfx/Filter/Metadata.hpp
    Gfx/Filter/Process.hpp
    Gfx/Filter/Layer.hpp
    Gfx/Filter/Library.hpp
    Gfx/Filter/PreviewWidget.hpp

    Gfx/Video/Executor.hpp
    Gfx/Video/Inspector.hpp
    Gfx/Video/Metadata.hpp
    Gfx/Video/Presenter.hpp
    Gfx/Video/Process.hpp
    Gfx/Video/View.hpp
    Gfx/Video/Layer.hpp

    Gfx/Images/Executor.hpp
    Gfx/Images/Metadata.hpp
    Gfx/Images/Process.hpp
    Gfx/Images/Layer.hpp
    Gfx/Images/ImageListChooser.hpp

    Gfx/Text/Executor.hpp
    Gfx/Text/Metadata.hpp
    Gfx/Text/Process.hpp
    Gfx/Text/Layer.hpp

    Gfx/Graph/Window.hpp
    Gfx/Graph/RenderState.hpp
    Gfx/Graph/Node.hpp
    Gfx/Graph/ISFNode.hpp
    Gfx/Graph/RenderedISFNode.hpp
    Gfx/Graph/Graph.hpp
    Gfx/Graph/Uniforms.hpp
    Gfx/Graph/Mesh.hpp
    Gfx/Graph/RenderList.hpp
    Gfx/Graph/NodeRenderer.hpp
    Gfx/Graph/VideoNode.hpp
    Gfx/Graph/VideoNodeRenderer.hpp
    Gfx/Graph/PhongNode.hpp
    Gfx/Graph/ImageNode.hpp
    Gfx/Graph/TexgenNode.hpp
    Gfx/Graph/ScreenNode.hpp
    Gfx/Graph/TextNode.hpp
    Gfx/Graph/ShaderCache.hpp
    Gfx/Graph/Utils.hpp
    Gfx/Graph/decoders/GPUVideoDecoder.hpp
    Gfx/Graph/decoders/HAP.hpp
    Gfx/Graph/decoders/RGBA.hpp
    Gfx/Graph/decoders/YUV420.hpp
    Gfx/Graph/decoders/YUV422.hpp
    Gfx/Graph/decoders/YUYV422.hpp

    Gfx/Settings/Model.hpp
    Gfx/Settings/Presenter.hpp
    Gfx/Settings/View.hpp
    Gfx/Settings/Factory.hpp

    Gfx/GfxApplicationPlugin.hpp
    Gfx/GfxContext.hpp
    Gfx/GfxExecNode.hpp
    Gfx/GfxExecContext.hpp
    Gfx/GfxParameter.hpp
    Gfx/GfxDevice.hpp
    Gfx/TexturePort.hpp
    Gfx/ShaderProgram.hpp

    Gfx/CameraDevice.hpp
    Gfx/WindowDevice.hpp

    # Gfx/graph/hap/source/hap.h

    score_plugin_gfx.hpp
)

set(SRCS
    ${SPOUT_SRCS}
    ${KINECT2_SRCS}

    Gfx/Filter/Executor.cpp
    Gfx/Filter/Library.cpp
    Gfx/Filter/Process.cpp
    Gfx/Filter/PreviewWidget.cpp

    Gfx/Video/Executor.cpp
    Gfx/Video/Inspector.cpp
    Gfx/Video/Presenter.cpp
    Gfx/Video/Process.cpp
    Gfx/Video/View.cpp

    Gfx/Images/Executor.cpp
    Gfx/Images/Process.cpp
    Gfx/Images/ImageListChooser.cpp

    Gfx/Text/Executor.cpp
    Gfx/Text/Process.cpp

    Gfx/Graph/decoders/GPUVideoDecoder.cpp
    Gfx/Graph/Node.cpp
    Gfx/Graph/Graph.cpp
    Gfx/Graph/RenderList.cpp
    Gfx/Graph/Mesh.cpp
    Gfx/Graph/ImageNode.cpp
    Gfx/Graph/ISFNode.cpp
    Gfx/Graph/RenderedISFNode.cpp
    Gfx/Graph/VideoNode.cpp
    Gfx/Graph/VideoNodeRenderer.cpp
    Gfx/Graph/NodeRenderer.cpp
    Gfx/Graph/OutputNode.cpp
    Gfx/Graph/ScreenNode.cpp
    Gfx/Graph/TextNode.cpp
    Gfx/Graph/PhongNode.cpp
    Gfx/Graph/Utils.cpp
    Gfx/Graph/Window.cpp

    Gfx/GfxApplicationPlugin.cpp
    Gfx/GfxExecNode.cpp
    Gfx/GfxContext.cpp
    Gfx/GfxDevice.cpp
    Gfx/TexturePort.cpp
    Gfx/ShaderProgram.cpp

    Gfx/CameraDevice.cpp
    Gfx/WindowDevice.cpp

    Gfx/Settings/Model.cpp
    Gfx/Settings/Presenter.cpp
    Gfx/Settings/View.cpp

    score_plugin_gfx.cpp
)


# Creation of the library
add_library(${PROJECT_NAME} ${SRCS} ${HDRS})

# Code generation
score_generate_command_list_file(${PROJECT_NAME} "${HDRS}")

# Link
target_link_libraries(${PROJECT_NAME} PUBLIC
  score_lib_base  score_lib_localtree score_lib_process score_plugin_dataflow score_plugin_engine
   ${QT_PREFIX}::ShaderTools ${QT_PREFIX}::ShaderToolsPrivate  ${QT_PREFIX}::GuiPrivate
   snappy # for HAP
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC
    isf
  PRIVATE
    avcodec avformat swresample avutil avdevice score_plugin_media
)

if(APPLE)
    find_library(QuartzCore_FK QuartzCore)
    find_library(AppKit_FK AppKit)
    find_library(OpenGL_FK OpenGL)
    find_library(Foundation_FK Foundation)
    find_library(AVFoundation_FK AVFoundation)
    find_library(CoreVideo_FK CoreVideo)
    find_library(CoreMedia_FK CoreMedia)
    find_library(VideoDecodeAcceleration_FK VideoDecodeAcceleration)
    find_library(CoreFoundation_FK CoreFoundation)
    find_library(AudioToolbox_FK AudioToolbox)
    find_library(VideoToolbox_FK VideoToolbox)
    find_package(BZip2 REQUIRED)
    find_package(ZLIB REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${QuartzCore_FK}
        ${AppKit_FK}
        ${OpenGL_FK}
        ${Foundation_FK}
        ${AVFoundation_FK}
        ${CoreVideo_FK}
        ${CoreMedia_FK}
        ${VideoDecodeAcceleration_FK}
        ${CoreFoundation_FK}
        ${AudioToolbox_FK}
        ${VideoToolbox_FK}
        BZip2::BZip2
        ZLIB::ZLIB
        )
endif()

if(WIN32)
#   add_subdirectory("${3RDPARTY_FOLDER}/Spout" "${CMAKE_CURRENT_BINARY_DIR}/Spout")
#   target_link_libraries(${PROJECT_NAME} PRIVATE Spout)
#   target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_SPOUT)
endif()

if(freenect2_FOUND)
  target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_FREENECT2)
  target_link_libraries(${PROJECT_NAME} PRIVATE ${freenect2_LIBRARIES})
endif()

if(TARGET ${QT_PREFIX}::OpenGLWidgets)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE ${QT_PREFIX}::OpenGLWidgets
  )
endif()
# Target-specific options
setup_score_plugin(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE -std=c++2a)

if(SCORE_INSTALL_HEADERS)
  install(
    FILES
      Gfx/Qt5CompatPush
      Gfx/Qt5CompatPop
    DESTINATION include/score/Gfx
    COMPONENT Devel
    OPTIONAL
  )
endif()

if(SCORE_ISF_TESTER)
add_executable(ISFTester tests/ISFTester.cpp)
target_link_libraries(ISFTester PRIVATE score_plugin_gfx)
endif()

if(SCORE_VIDEO_TESTER)
add_executable(VideoTester tests/VideoTester.cpp)
target_link_libraries(VideoTester PRIVATE score_plugin_gfx)
endif()

