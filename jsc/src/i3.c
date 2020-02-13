#include <JavaScriptCore/JavaScript.h>
#include <gtk/gtk.h>
#include <i3ipc-glib/i3ipc-glib.h>
#include <stdio.h>
#include <webkit2/webkit-web-extension.h>

i3ipcConnection *conn;
JSGlobalContextRef jsContext;

static void update_client_workspaces() {
  gchar *workspaces;
  workspaces = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_GET_WORKSPACES,
                                        NULL, NULL);
  char script[1352]; // size of get_workspace JSON for 10 active workspaces
  sprintf(script, "updateWorkspaces(%s)", workspaces);
  JSStringRef jsScript = JSStringCreateWithUTF8CString(script);
  JSEvaluateScript(jsContext, jsScript, NULL, NULL, 0, NULL);
}

static char *JSValue_to_char(JSContextRef context, JSValueRef value) {
  size_t len;
  char *cstr;
  JSStringRef jsstr = JSValueToStringCopy(context, value, NULL);
  len = JSStringGetMaximumUTF8CStringSize(jsstr);
  cstr = g_new(char, len);
  JSStringGetUTF8CString(jsstr, cstr, len);
  JSStringRelease(jsstr);

  return cstr;
}

static void i3_init_cb(JSContextRef ctx, JSObjectRef object) {
  GError *err = NULL;
  i3ipcCommandReply *cr;
  conn = i3ipc_connection_new(NULL, &err);
  cr = i3ipc_connection_subscribe(conn, I3IPC_EVENT_WORKSPACE, &err);
  g_signal_connect(conn, "workspace", G_CALLBACK(update_client_workspaces),
                   NULL);
  i3ipc_command_reply_free(cr);
}

static void i3_destroy_cb(JSObjectRef object) { g_object_unref(conn); }

static JSValueRef i3_msg_cb(JSContextRef context, JSObjectRef function,
                            JSObjectRef thisObject, size_t argumentCount,
                            const JSValueRef arguments[],
                            JSValueRef *exception) {
  if (argumentCount == 1 && JSValueIsString(context, arguments[0])) {
    char *cstr = JSValue_to_char(context, arguments[0]);

    gchar *reply;
    reply =
        i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_COMMAND, cstr, NULL);

    printf("\nReply: %s\n", reply);

    g_free(reply);
    g_free(cstr);
  }

  return JSValueMakeUndefined(context);
}

static JSValueRef i3_workspaces_cb(JSContextRef context, JSObjectRef function,
                                   JSObjectRef thisObject, size_t argumentCount,
                                   const JSValueRef arguments[],
                                   JSValueRef *exception) {
  gchar *wsJSON;
  JSStringRef strref;

  wsJSON = i3ipc_connection_message(conn, I3IPC_MESSAGE_TYPE_GET_WORKSPACES,
                                    NULL, NULL);
  strref = JSStringCreateWithUTF8CString(wsJSON);

  g_free(wsJSON);

  return JSValueMakeString(context, strref);
}

/* Class method declarations */
static const JSStaticFunction i3_staticfuncs[] = {
    {"msg", i3_msg_cb, kJSPropertyAttributeReadOnly},
    {"workspaces", i3_workspaces_cb, kJSPropertyAttributeReadOnly},
    {NULL, NULL, 0}};

static const JSClassDefinition i3_def = {
    0,                     // version
    kJSClassAttributeNone, // attributes
    "I3",                  // className
    NULL,                  // parentClass
    NULL,                  // staticValues
    i3_staticfuncs,        // staticFunctions
    i3_init_cb,            // initialize
    i3_destroy_cb,         // finalize
    NULL,                  // hasProperty
    NULL,                  // getProperty
    NULL,                  // setProperty
    NULL,                  // deleteProperty
    NULL,                  // getPropertyNames
    NULL,                  // callAsFunction
    NULL,                  // callAsConstructor
    NULL,                  // hasInstance
    NULL                   // convertToType
};

static void window_object_cleared_callback(WebKitScriptWorld *world,
                                           WebKitWebPage *web_page,
                                           WebKitFrame *frame,
                                           gpointer user_data) {
  jsContext =
      webkit_frame_get_javascript_context_for_script_world(frame, world);

  JSClassRef classDef = JSClassCreate(&i3_def);
  JSObjectRef classObj = JSObjectMake(jsContext, classDef, jsContext);
  JSObjectRef globalObj = JSContextGetGlobalObject(jsContext);
  JSStringRef str = JSStringCreateWithUTF8CString("I3");
  JSObjectSetProperty(jsContext, globalObj, str, classObj,
                      kJSPropertyAttributeNone, NULL);
}

G_MODULE_EXPORT void
webkit_web_extension_initialize(WebKitWebExtension *extension) {
  g_signal_connect(webkit_script_world_get_default(), "window-object-cleared",
                   G_CALLBACK(window_object_cleared_callback), NULL);
}