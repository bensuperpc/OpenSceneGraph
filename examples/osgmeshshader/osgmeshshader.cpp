// This is public domain software and comes with
// absolutely no warranty. Use of public domain software
// may vary between counties, but in general you are free
// to use and distribute this software for any purpose.


// Example: OSG using an OpenGL 3.1 context.
// The comment block at the end of the source describes building OSG
// for use with OpenGL 3.x.

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/GraphicsContext>
#include <osg/Camera>
#include <osg/Viewport>
#include <osg/StateSet>
#include <osg/Program>
#include <osg/Shader>
#include <osgUtil/Optimizer>


class DrawMeshTasks : public osg::Drawable
{
public:

    DrawMeshTasks() :
        first(0),
        count(0)
    {
    }

    DrawMeshTasks(GLuint in_first, GLuint in_count) :
        first(in_first),
        count(in_count)
    {
    }

    GLuint first;
    GLuint count;

    virtual void drawImplementation(osg::RenderInfo& renderInfo) const
    {
        const osg::GLExtensions* extensions = renderInfo.getState()->get<osg::GLExtensions>();

        void* (* my_glXGetProcAddress) (const GLchar *name);
        osg::setGLExtensionFuncPtr(my_glXGetProcAddress, "glXGetProcAddress", "glXGetProcAddressARB");

        void (GL_APIENTRY * my_glDrawMeshTasksNV) (GLuint first, GLuint count);

        osg::convertPointer(my_glDrawMeshTasksNV, my_glXGetProcAddress("glDrawMeshTasksNV"));

        if (extensions->isMeshShaderSupported && my_glDrawMeshTasksNV)
        {
            my_glDrawMeshTasksNV(first, count);
        }
        else
        {
            OSG_NOTICE<<"glDrawMeshTasksNV not supported. "<<std::endl;
        }
    }
};

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    static const char* meshSource = \
        "#version 450 \n"
        "#extension GL_NV_mesh_shader : enable\n"
        "layout(local_size_x = 3) in;"
        "layout(max_vertices = 64) out;"
        "layout(max_primitives = 126) out;"
        "layout(triangles) out;"
        "const vec3 vertices[3] = {vec3(-1,-1,0), vec3(1,-1,0), vec3(0,1,0)};"
        "void main()"
        "{"
            "uint id = gl_LocalInvocationID.x;"
            "gl_MeshVerticesNV[id].gl_Position = vec4(vertices[id], 2);"
            "gl_PrimitiveIndicesNV[id] = id;"
            "gl_PrimitiveCountNV = 1;"
        "}";


    static const char* fragmentSource = \
        "#version 450 \n"
        "#extension GL_NV_fragment_shader_barycentric : enable\n"
        "out vec4 color;"
        "void main()"
        "{"
            "color = vec4(gl_BaryCoordNV, 1.0);"
        "}";

    osg::ref_ptr<osg::Shader> vShader = new osg::Shader( osg::Shader::MESH, meshSource );
    osg::ref_ptr<osg::Shader> fShader = new osg::Shader( osg::Shader::FRAGMENT, fragmentSource );

    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( vShader.get() );
    program->addShader( fShader.get() );

    osg::ref_ptr<osg::Node> drawMesh = new DrawMeshTasks(0, 1);
    drawMesh->getOrCreateStateSet()->setAttribute( program.get() );

    osgViewer::Viewer viewer(arguments);

    viewer.setSceneData( drawMesh );

    // for non GL3/GL4 and non GLES2 platforms we need enable the osg_ uniforms that the shaders will use,
    // you don't need thse two lines on GL3/GL4 and GLES2 specific builds as these will be enable by default.
    //gc->getState()->setUseModelViewAndProjectionUniforms(true);
    //gc->getState()->setUseVertexAttributeAliasing(true);

    return( viewer.run() );
}