#include <mruby.h>
#include <mruby/array.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/string.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glext.h>
#include <string.h>
#include <stdlib.h>
#include "../../../deps/pugl/pugl/event.h"
#include "../../../deps/pugl/pugl/common.h"
#include "../../../deps/pugl/pugl/pugl.h"
#include "../../../deps/rtosc/include/rtosc/rtosc.h"
#include "../../../deps/osc-bridge/src/gem.h"

static mrb_value
mrb_gl_viewport(mrb_state *mrb, mrb_value self)
{
    mrb_float x, y, w, h;
    mrb_get_args(mrb, "ffff", &x, &y, &w, &h);
    glViewport(x, y, w, h);
    return self;
}
static mrb_value
mrb_gl_clear_color(mrb_state *mrb, mrb_value self)
{
    mrb_float r, b, g, a;
    mrb_get_args(mrb, "ffff", &r, &b, &g, &a);
    glClearColor(r, b, g, a);
    return self;
}
static mrb_value
mrb_gl_clear(mrb_state *mrb, mrb_value self)
{
    mrb_int clear_mode;
    mrb_get_args(mrb, "i", &clear_mode);
    //glClear(clear_mode);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    return self;
}

static mrb_value
mrb_gl_scissor(mrb_state *mrb, mrb_value self)
{
    mrb_float x, y, w, h;
    mrb_get_args(mrb, "ffff", &x, &y, &w, &h);
    glEnable(GL_SCISSOR_TEST);
    glScissor(x, y, w, h);
    return self;
}

static mrb_value
mrb_gl_scissor_end(mrb_state *mrb, mrb_value self)
{
    glDisable(GL_SCISSOR_TEST);
    return self;
}

/*******************************************************************************
 *                          PUGL Code Here                                     *
 *                                                                             *
 ******************************************************************************/
static void
onReshape(PuglView* view, int width, int height)
{
    void **v = (void**)puglGetHandle(view);
    //printf("reshape to %dx%d\n", width, height);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "resize", 2, mrb_fixnum_value(width), mrb_fixnum_value(height));
    }
#if 0
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, width, height);
	gluPerspective(45.0f, width/(float)height, 1.0f, 10.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
#endif
}

static void
onDisplay(PuglView* view)
{
    void **v = (void**)puglGetHandle(view);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "draw", 0);
    }
	//puglPostRedisplay(view);
}

static void
printModifiers(PuglView* view)
{
	int mods = puglGetModifiers(view);
    (void) mods;
	//fprintf(stderr, "Modifiers:%s%s%s%s\n",
	//        (mods & PUGL_MOD_SHIFT) ? " Shift"   : "",
	//        (mods & PUGL_MOD_CTRL)  ? " Ctrl"    : "",
	//        (mods & PUGL_MOD_ALT)   ? " Alt"     : "",
	//        (mods & PUGL_MOD_SUPER) ? " Super" : "");
}

static void
onEvent(PuglView* view, const PuglEvent* event)
{
	if (event->type == PUGL_KEY_PRESS) {
		const uint32_t ucode = event->key.character;
        (void) ucode;
		//fprintf(stderr, "Key %u (char %u) down (%s)%s\n",
		//        event->key.keycode, ucode, event->key.utf8,
		//        event->key.filter ? " (filtered)" : "");
	}
}

static void
onSpecial(PuglView* view, bool press, PuglKey key)
{
	//fprintf(stderr, "Special key %d %s ", key, press ? "down" : "up");
	//printModifiers(view);
}

static void
onMotion(PuglView* view, int x, int y)
{
	//fprintf(stderr, "Mouse Move %d %d\n", x, y);
    void **v = (void**)puglGetHandle(view);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "cursor", 2, mrb_fixnum_value(x), mrb_fixnum_value(y));
    }
}

static void
onMouse(PuglView* view, int button, bool press, int x, int y)
{
    void **v = (void**)puglGetHandle(view);
	//fprintf(stderr, "Mouse %d %s at %d,%d ",
	//        button, press ? "down" : "up", x, y);
	//printModifiers(view);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "mouse", 4, 
                mrb_fixnum_value(button),
                mrb_fixnum_value(press),
                mrb_fixnum_value(x),
                mrb_fixnum_value(y));
    }
}

static void
onScroll(PuglView* view, int x, int y, float dx, float dy)
{
	//fprintf(stderr, "Scroll %d %d %f %f ", x, y, dx, dy);
	printModifiers(view);
	//dist += dy / 4.0f;
}

static void
onClose(PuglView* view)
{
    void **v = (void**)puglGetHandle(view);
    if(v) {
        mrb_value obj = mrb_obj_value(v[1]);
        mrb_funcall(v[0], obj, "quit", 0);
    }
}

static void
mrb_pugl_free(mrb_state *mrb, void *ptr)
{
    printf("================ FFFFFFFFFFFFFFRRRRRRRREEEEEEEEEEEE\n");
}

const struct mrb_data_type mrb_pugl_type = {"PUGL", mrb_pugl_free};

static mrb_value
mrb_pugl_tick(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    puglProcessEvents(view);
    return self;
}


static mrb_value
mrb_pugl_initialize(mrb_state *mrb, mrb_value self)
{
    PuglView *view = puglInit(0,0);
    //puglInitWindowClass(view, "PuglWindow");
	puglInitWindowSize(view, 1181, 659);
    puglInitResizable(view, true);

	puglSetEventFunc(view, onEvent);
	puglSetMotionFunc(view, onMotion);
	puglSetMouseFunc(view, onMouse);
	puglSetScrollFunc(view, onScroll);
	puglSetSpecialFunc(view, onSpecial);
	puglSetDisplayFunc(view, onDisplay);
	puglSetReshapeFunc(view, onReshape);
	puglSetCloseFunc(view, onClose);

	puglCreateWindow(view, "Pugl Test");
	puglShowWindow(view);
    puglProcessEvents(view);

    mrb_data_init(self, view, &mrb_pugl_type);
    mrb_funcall(mrb, self, "w=", 1, mrb_fixnum_value(1181));
    mrb_funcall(mrb, self, "h=", 1, mrb_fixnum_value(659));

    return self;
}

static mrb_value
mrb_pugl_size(mrb_state *mrb, mrb_value self)
{
    mrb_value ary = mrb_ary_new(mrb);
    mrb_sym xid = mrb_intern_str(mrb, mrb_str_new_cstr(mrb, "x"));
    mrb_sym yid = mrb_intern_str(mrb, mrb_str_new_cstr(mrb, "y"));
    mrb_ary_push(mrb, ary, mrb_attr_get(mrb, self, xid));
    mrb_ary_push(mrb, ary, mrb_attr_get(mrb, self, yid));
    return ary;
}

static mrb_value
mrb_pugl_size_set(mrb_state *mrb, mrb_value self)
{
    mrb_value ary;
    mrb_get_args(mrb, "o", &ary);
    mrb_value w = mrb_ary_ref(mrb, ary, 0);
    mrb_value h = mrb_ary_ref(mrb, ary, 1);
    mrb_funcall(mrb, self, "w=", 1, w);
    mrb_funcall(mrb, self, "h=", 1, h);
    return self;
}

static mrb_value
mrb_pugl_make_current(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    void puglEnterContext(PuglView* view);
    puglEnterContext(view);
    return self;
}

static mrb_value
mrb_pugl_should_close(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    //void puglEnterContext(PuglView* view);
    //puglEnterContext(view);
    return mrb_false_value();
}

static mrb_value
mrb_pugl_poll(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    puglProcessEvents(view);
    return mrb_false_value();
}


static mrb_value
mrb_pugl_impl(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    mrb_value zrunner;
    mrb_get_args(mrb, "o", &zrunner);
    void **v = mrb_malloc(mrb, 2*sizeof(void*));
    v[0] = mrb;
    v[1] = mrb_cptr(zrunner);
    puglSetHandle(view, v);
    return mrb_false_value();
}

static mrb_value
mrb_pugl_dummy(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
    //void puglEnterContext(PuglView* view);
    //puglEnterContext(view);
    return self;
}

static mrb_value
mrb_pugl_refresh(mrb_state *mrb, mrb_value self)
{
    PuglView *view = (PuglView*)mrb_data_get_ptr(mrb, self, &mrb_pugl_type);
	puglPostRedisplay(view);
    return self;
}

/*****************************************************************************
 *                         GL Buffer Code                                    *
 *****************************************************************************/

typedef struct {
    int w, h;
	GLuint fbo;
	GLuint rbo;
	GLuint texture;
} GLframebuffer;

static int
createFBO(int w, int h, GLframebuffer *fb)
{
    /* texture */
    glGenTextures(1, &fb->texture);
    glBindTexture(GL_TEXTURE_2D, fb->texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* frame buffer object */
    glGenFramebuffers(1, &fb->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb->fbo);

    /* render buffer object */
    glGenRenderbuffers(1, &fb->rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb->rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);

    /* combine all */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, fb->texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, fb->rbo);

    return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
}

static void mrb_fbo_free(mrb_state *mrb, void *ptr);
const struct mrb_data_type mrb_fbo_type = {"FBO", mrb_fbo_free};
static void
mrb_fbo_free(mrb_state *mrb, void *ptr)
{
    mrb_assert(mrb && false);
    GLframebuffer *fbo = (GLframebuffer *)ptr;
    glDeleteRenderbuffers(1, &fbo->rbo);
    glDeleteFramebuffers(1, &fbo->fbo);
    glDeleteTextures(1, &fbo->texture);
}

static mrb_value
mrb_fbo_initialize(mrb_state *mrb, mrb_value self)
{
    mrb_int w, h;
    mrb_get_args(mrb, "ii", &w, &h);
    GLframebuffer *fbo = mrb_malloc(mrb, sizeof(GLframebuffer));
    fbo->w   = w;
    fbo->h   = h;
    fbo->fbo = 0;
    fbo->rbo = 0;
    fbo->texture = 0;
    createFBO(w, h, fbo);
    mrb_data_init(self, fbo, &mrb_fbo_type);
    return self;
}

static mrb_value
mrb_fbo_deselect(mrb_state *mrb, mrb_value self)
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return self;
}

//static mrb_value
//mrb_fbo_copy_region(mrb_state *mrb, mrb_value self)
//{
//    //Disable rendering to the color buffer
//    glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
//    //Start using the stencil
//    glEnable( GL_STENCIL_TEST );
//
//    //Place a 1 where rendered
//    glStencilFunc( GL_ALWAYS, 1, 1 );
//    //Replace where rendered
//    glStencilOp( GL_REPLACE, GL_REPLACE, GL_REPLACE );
//    //Render stencil triangle
//    glTranslatef( gPolygonX, gPolygonY, 0.f );
//    glRotatef( gPolygonAngle, 0.f, 0.f, 1.f );
//    glBegin( GL_TRIANGLES );
//    glVertex2f( -0.f / 4.f, -SCREEN_HEIGHT / 4.f );
//    glVertex2f( SCREEN_WIDTH / 4.f, SCREEN_HEIGHT / 4.f );
//    glVertex2f( -SCREEN_WIDTH / 4.f, SCREEN_HEIGHT / 4.f );
//    glEnd();
//    //Reenable color
//    glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
//    //Where a 1 was not rendered
//    glStencilFunc(GL_NOTEQUAL, 1, 1 );
//    //Keep the pixel
//    glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
//}

const struct mrb_data_type mrb_nvg_context_type;
typedef void NVGcontext;
int nvglCreateImageFromHandleGL2(NVGcontext* ctx, GLuint textureId, int w, int h, int imageFlags);

static mrb_value
mrb_fbo_image(mrb_state *mrb, mrb_value self)
{
    mrb_value      obj;
    mrb_get_args(mrb, "o", &obj);
    NVGcontext    *ctx = mrb_data_get_ptr(mrb, obj, &mrb_nvg_context_type);
    GLframebuffer *fbo = (GLframebuffer*)mrb_data_get_ptr(mrb, self, &mrb_fbo_type);

    return mrb_fixnum_value(nvglCreateImageFromHandleGL2(ctx, fbo->texture, fbo->w, fbo->h, (1<<2)|(1<<3)));
}

static mrb_value
mrb_fbo_select(mrb_state *mrb, mrb_value self)
{
    GLframebuffer *fbo = (GLframebuffer*)mrb_data_get_ptr(mrb, self, &mrb_fbo_type);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
    return self;
}


/*****************************************************************************
 *                      Remote Parameter Code                                *
 *****************************************************************************/
typedef struct {
    mrb_state *mrb;
    mrb_value  cb;
} remote_cb_data;

typedef struct {
    bridge_t        *br;
    uri_t            uri;
    char             type;
    int              cbs;
    remote_cb_data **cb_refs;
} remote_param_data;

typedef struct {
    bridge_t *br;
    schema_t  sch;
} remote_data;


void
mrb_remote_metadata_free(mrb_state *mrb, void *ptr)
{
    printf("================ metadata FFFFFFFFFFFFFFRRRRRRRREEEEEEEEEEEE\n");
}

void
mrb_remote_free(mrb_state *mrb, void *ptr)
{
    printf("================ remote FFFFFFFFFFFFFFRRRRRRRREEEEEEEEEEEE\n");
}

static void remote_cb(const char *msg, void *data);

void
mrb_remote_param_free(mrb_state *mrb, void *ptr)
{
    remote_param_data *data = (remote_param_data*)ptr;
    for(int i=0; i<data->cbs; ++i) {
        remote_cb_data *ref = data->cb_refs[i];
        br_del_callback(data->br, data->uri, remote_cb, ref);
    }
}

const struct mrb_data_type mrb_remote_type          = {"Remote", mrb_remote_free};
const struct mrb_data_type mrb_remote_metadata_type = {"RemoteMetadata", mrb_remote_metadata_free};
const struct mrb_data_type mrb_remote_param_type    = {"RemoteParam", mrb_remote_param_free};

static mrb_value
mrb_remote_initalize(mrb_state *mrb, mrb_value self)
{
    remote_data *data = mrb_malloc(mrb, sizeof(remote_data));
    data->br  = br_create("localhost:1337");
    data->sch = br_get_schema(data->br, "");

    mrb_data_init(self, data, &mrb_remote_type);
    return self;
}

static mrb_value
mrb_remote_tick(mrb_state *mrb, mrb_value self)
{
    remote_data *data = (remote_data*)mrb_data_get_ptr(mrb, self, &mrb_remote_type);
    br_tick(data->br);
    return self;
}

static mrb_value
mrb_remote_metadata_initalize(mrb_state *mrb, mrb_value self)
{
    mrb_value remote;
    mrb_value path;
    mrb_get_args(mrb, "oS", &remote, &path);

    //Obtain the schema handle
    remote_data *data = (remote_data*)mrb_data_get_ptr(mrb, remote, &mrb_remote_type);
    bridge_t *br = data->br;
    schema_t sch = data->sch;
    schema_handle_t handle = sm_get(sch, mrb_string_value_ptr(mrb, path));
    mrb_value opts = mrb_nil_value();
    if(handle.opts) {
        opts = mrb_ary_new(mrb);
        for(int i=0; i<handle.opts->num_opts; ++i)
        {
            mrb_value opt = mrb_ary_new(mrb);
            mrb_ary_push(mrb, opt, mrb_fixnum_value(handle.opts->ids[i]));
            mrb_ary_push(mrb, opt, mrb_str_new_cstr(mrb, handle.opts->labels[i]));
            mrb_ary_push(mrb, opts, opt);
        }
    }

#define setfield(x, cstr) \
    mrb_funcall(mrb, self, x, 1, \
                      mrb_str_new_cstr(mrb, cstr))
#define setfield2(x, value) \
    mrb_funcall(mrb, self, x, 1, value)
    setfield("name=",       sm_get_name(handle));
    setfield("short_name=", sm_get_short(handle));
    setfield("tooltip=",    sm_get_tooltip(handle));
    setfield2("options=",   opts);
#undef setfield
#undef setfield2
    return self;
}

static void
remote_cb_127(const char *msg, remote_cb_data *cb)
{
    //assume the 0..127 integer case for the input
    //assume the 0..1   float   case for the output
    
    mrb_assert(!strcmp("i",rtosc_argument_string(msg)));
    int arg = rtosc_argument(msg, 0).i;

    mrb_assert(0 <= arg && arg <= 127);

    mrb_float cb_val = arg/127.0;

    mrb_funcall(cb->mrb, cb->cb, "call", 1, mrb_float_value(cb->mrb,cb_val));
}

static void
remote_cb_fvec(const char *msg, remote_cb_data *cb)
{
    mrb_value ary = mrb_ary_new(cb->mrb);
    for(rtosc_arg_itr_t itr = rtosc_itr_begin(msg); !rtosc_itr_end(itr);) {
        rtosc_arg_val_t val = rtosc_itr_next(&itr);
        if(val.type == 'f')
            mrb_ary_push(cb->mrb, ary, mrb_float_value(cb->mrb, val.val.f));
    }

    mrb_funcall(cb->mrb, cb->cb, "call", 1, ary);
}

static void
remote_cb(const char *msg, void *data)
{
    remote_cb_data *cb = (remote_cb_data*) data;
    if(!strcmp("i", rtosc_argument_string(msg)))
        remote_cb_127(msg, cb);
    else if(!strcmp("f", rtosc_argument_string(msg)))
        mrb_funcall(cb->mrb, cb->cb, "call", 1, mrb_float_value(cb->mrb,rtosc_argument(msg, 0).f));
    else
        remote_cb_fvec(msg, cb);
}


static mrb_value
mrb_remote_param_initalize(mrb_state *mrb, mrb_value self)
{
    mrb_value remote;
    mrb_value uri;
    mrb_get_args(mrb, "oS", &remote, &uri);
    remote_param_data *data = mrb_malloc(mrb, sizeof(remote_param_data));
    remote_data *rdata = (remote_data*)mrb_data_get_ptr(mrb, remote, &mrb_remote_type);
    data->br  = rdata->br;
    data->uri = strdup(mrb_string_value_ptr(mrb, uri));
    data->type = 'i';
    data->cb_refs = NULL;
    data->cbs     = 0;
    if(strstr(data->uri, "Pfreq"))
        data->type = 'f';

    mrb_funcall(mrb, self, "remote=", 1, remote);
    mrb_data_init(self, data, &mrb_remote_param_type);
    return self;
}

static mrb_value
mrb_remote_param_set_callback(mrb_state *mrb, mrb_value self)
{
    remote_param_data *param = (remote_param_data*)
        mrb_data_get_ptr(mrb, self, &mrb_remote_param_type);

    remote_cb_data *data = malloc(sizeof(remote_cb_data));
    data->mrb = mrb;
    mrb_get_args(mrb, "o", &data->cb);

    mrb_funcall(mrb, self, "add_cb", 1, data->cb);

    mrb_assert(param->br);
    mrb_assert(param->uri);
    br_add_callback(param->br, param->uri, remote_cb, data);
    param->cbs += 1;
    param->cb_refs = realloc(param->cb_refs, param->cbs*sizeof(void*));
    param->cb_refs[param->cbs-1] = data;


    param = (remote_param_data*) mrb_data_get_ptr(mrb, self, &mrb_remote_param_type);
    return self;
}

static mrb_value
mrb_remote_param_set_value(mrb_state *mrb, mrb_value self)
{
    remote_param_data *param;
    param = (remote_param_data*) mrb_data_get_ptr(mrb, self, &mrb_remote_param_type);
    mrb_assert(param);

    mrb_float value = 0;
    mrb_get_args(mrb, "f", &value);
    mrb_assert(param);
    mrb_assert(param->br);
    mrb_assert(param->uri);

    if(param->type == 'i') {
        int next = (127.0*value);
        br_set_value_int(param->br, param->uri, next);
    } else if(param->type == 'f') {
        br_set_value_float(param->br, param->uri, value);
    }
    return self;
}

static mrb_value
mrb_remote_param_display_value(mrb_state *mrb, mrb_value self)
{
    remote_param_data *param;
    param = (remote_param_data*) mrb_data_get_ptr(mrb, self, &mrb_remote_param_type);
    mrb_assert(param);

    bridge_t *br = param->br;
    //try to see if the value is already in the cache
    for(int i=0; i<br->cache_len; ++i) {
        if(!strcmp(br->cache[i].path, param->uri) &&
                br->cache[i].valid) {
            if(br->cache[i].type == 'i')
                return mrb_fixnum_value(br->cache[i].val.i);
        }
    }

    return mrb_nil_value();
}

static mrb_value
mrb_remote_param_refresh(mrb_state *mrb, mrb_value self)
{
    remote_param_data *param;
    param = (remote_param_data*) mrb_data_get_ptr(mrb, self, &mrb_remote_param_type);
    mrb_assert(param);

    br_refresh(param->br, param->uri);
    return self;
}

static mrb_value
mrb_remote_param_watch(mrb_state *mrb, mrb_value self)
{
    remote_param_data *param;
    param = (remote_param_data*) mrb_data_get_ptr(mrb, self, &mrb_remote_param_type);
    mrb_assert(param);

    br_watch(param->br, param->uri);
    return self;
}


// Puting it all together
void
mrb_mruby_widget_lib_gem_init(mrb_state* mrb) {
    struct RClass *module = mrb_define_module(mrb, "GL");
    mrb_define_class_method(mrb, module, "gl_viewport",    mrb_gl_viewport,    MRB_ARGS_REQ(4));
    mrb_define_class_method(mrb, module, "gl_clear_color", mrb_gl_clear_color, MRB_ARGS_REQ(4));
    mrb_define_class_method(mrb, module, "gl_clear",       mrb_gl_clear,       MRB_ARGS_REQ(1));
    mrb_define_class_method(mrb, module, "gl_scissor",     mrb_gl_scissor,     MRB_ARGS_REQ(4));
    mrb_define_class_method(mrb, module, "gl_scissor_end", mrb_gl_scissor_end, MRB_ARGS_REQ(0));


    struct RClass *pugl = mrb_define_class_under(mrb, module, "PUGL", mrb->object_class);
    MRB_SET_INSTANCE_TT(pugl, MRB_TT_DATA);

    mrb_define_method(mrb, pugl, "initialize",   mrb_pugl_initialize,   MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "make_current", mrb_pugl_make_current, MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "should_close", mrb_pugl_should_close, MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "size",         mrb_pugl_size,         MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "size=",        mrb_pugl_size_set,     MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "title=",       mrb_pugl_dummy,        MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "poll",         mrb_pugl_poll,         MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "impl=",        mrb_pugl_impl,         MRB_ARGS_REQ(1));
    mrb_define_method(mrb, pugl, "refresh",      mrb_pugl_refresh,      MRB_ARGS_NONE());
    mrb_define_method(mrb, pugl, "destroy",       mrb_pugl_dummy,        MRB_ARGS_NONE());

    struct RClass *fbo = mrb_define_class_under(mrb, module, "FBO",
            mrb->object_class);
    MRB_SET_INSTANCE_TT(fbo, MRB_TT_DATA);
    mrb_define_method(mrb, fbo, "initialize",   mrb_fbo_initialize,   MRB_ARGS_REQ(2));
    mrb_define_method(mrb, fbo, "select",       mrb_fbo_select,       MRB_ARGS_NONE());
    mrb_define_method(mrb, fbo, "deselect",     mrb_fbo_deselect,     MRB_ARGS_NONE());
    mrb_define_method(mrb, fbo, "image",        mrb_fbo_image,        MRB_ARGS_REQ(1));
    mrb_define_method(mrb, fbo, "destroy",      mrb_pugl_dummy,       MRB_ARGS_NONE());

    //Define the remote API
    struct RClass *osc = mrb_define_module(mrb, "OSC");
    struct RClass *remote = mrb_define_class_under(mrb, osc, "Remote", mrb->object_class);
    MRB_SET_INSTANCE_TT(remote, MRB_TT_DATA);
    mrb_define_method(mrb, remote, "initialize", mrb_remote_initalize, MRB_ARGS_NONE());
    mrb_define_method(mrb, remote, "tick",       mrb_remote_tick,      MRB_ARGS_NONE());

    struct RClass *metadata = mrb_define_class_under(mrb, osc, "RemoteMetadata", mrb->object_class);
    MRB_SET_INSTANCE_TT(metadata, MRB_TT_DATA);
    mrb_define_method(mrb, metadata, "initialize", mrb_remote_metadata_initalize, MRB_ARGS_REQ(2));

    struct RClass *param = mrb_define_class_under(mrb, osc, "RemoteParam", mrb->object_class);
    MRB_SET_INSTANCE_TT(param, MRB_TT_DATA);
    mrb_define_method(mrb, param, "initialize", mrb_remote_param_initalize, MRB_ARGS_REQ(2));
    mrb_define_method(mrb, param, "set_callback", mrb_remote_param_set_callback, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, param, "set_value",    mrb_remote_param_set_value, MRB_ARGS_REQ(1));
    mrb_define_method(mrb, param, "display_value",mrb_remote_param_display_value, MRB_ARGS_NONE());
    mrb_define_method(mrb, param, "refresh",      mrb_remote_param_refresh, MRB_ARGS_NONE());
    mrb_define_method(mrb, param, "watch",        mrb_remote_param_watch, MRB_ARGS_NONE());
}

void
mrb_mruby_widget_lib_gem_final(mrb_state* mrb) {
    /* finalizer */
}
