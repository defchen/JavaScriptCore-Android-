
#ifndef JNIUtility_h
#define JNIUtility_h

#include <jni.h>
#include <stdio.h>

// The order of these items can not be modified as they are tightly
// bound with the JVM on Mac OSX. If new types need to be added, they
// should be added to the end. It is used in jni_obc.mm when calling
// through to the JVM. Newly added items need to be made compatible
// in that file.
typedef enum {
    invalid_type = 0,
    void_type,
    object_type,
    boolean_type,
    byte_type,
    char_type,
    short_type,
    int_type,
    long_type,
    float_type,
    double_type,
    array_type
} JNIType;

namespace JSC {

namespace Bindings {

class JavaParameter;

const char* getCharactersFromJString(jstring);
void releaseCharactersForJString(jstring, const char*);

const char* getCharactersFromJStringInEnv(JNIEnv*, jstring);
void releaseCharactersForJStringInEnv(JNIEnv*, jstring, const char*);
const jchar* getUCharactersFromJStringInEnv(JNIEnv*, jstring);
void releaseUCharactersForJStringInEnv(JNIEnv*, jstring, const jchar*);

JNIType JNITypeFromClassName(const char* name);
JNIType JNITypeFromPrimitiveType(char type);
const char* signatureFromPrimitiveType(JNIType);

jvalue getJNIField(jobject, JNIType, const char* name, const char* signature);

jmethodID getMethodID(jobject, const char* name, const char* sig);
JNIEnv* getJNIEnv();
JavaVM* getJavaVM();
void setJavaVM(JavaVM*);


template <typename T> struct JNICaller;

template<> struct JNICaller<void> {
    static void callA(jobject obj, jmethodID mid, jvalue* args)
    {
        getJNIEnv()->CallVoidMethodA(obj, mid, args);
    }
    static void callV(jobject obj, jmethodID mid, va_list args)
    {
        getJNIEnv()->CallVoidMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jobject> {
    static jobject callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallObjectMethodA(obj, mid, args);
    }
    static jobject callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallObjectMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jboolean> {
    static jboolean callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallBooleanMethodA(obj, mid, args);
    }
    static jboolean callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallBooleanMethodV(obj, mid, args);
    }
    static jboolean callStaticV(jclass cls, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallStaticBooleanMethod(cls, mid, args);
    }
};

template<> struct JNICaller<jbyte> {
    static jbyte callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallByteMethodA(obj, mid, args);
    }
    static jbyte callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallByteMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jchar> {
    static jchar callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallCharMethodA(obj, mid, args);
    }
    static jchar callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallCharMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jshort> {
    static jshort callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallShortMethodA(obj, mid, args);
    }
    static jshort callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallShortMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jint> {
    static jint callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallIntMethodA(obj, mid, args);
    }
    static jint callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallIntMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jlong> {
    static jlong callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallLongMethodA(obj, mid, args);
    }
    static jlong callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallLongMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jfloat> {
    static jfloat callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallFloatMethodA(obj, mid, args);
    }
    static jfloat callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallFloatMethodV(obj, mid, args);
    }
};

template<> struct JNICaller<jdouble> {
    static jdouble callA(jobject obj, jmethodID mid, jvalue* args)
    {
        return getJNIEnv()->CallDoubleMethodA(obj, mid, args);
    }
    static jdouble callV(jobject obj, jmethodID mid, va_list args)
    {
        return getJNIEnv()->CallDoubleMethodV(obj, mid, args);
    }
};

template<typename T> T callJNIMethodIDA(jobject obj, jmethodID mid, jvalue *args)
{
    return JNICaller<T>::callA(obj, mid, args);
}

template<typename T>
static T callJNIMethodV(jobject obj, const char* name, const char* sig, va_list args)
{
    JavaVM* jvm = getJavaVM();
    JNIEnv* env = getJNIEnv();

    if (obj && jvm && env) {
        jclass cls = env->GetObjectClass(obj);
        if (cls) {
            jmethodID mid = env->GetMethodID(cls, name, sig);
            if (mid) {
                // Avoids references to cls without popping the local frame.
                env->DeleteLocalRef(cls);
                return JNICaller<T>::callV(obj, mid, args);
            }
            fprintf(stderr, "%s: Could not find method: %s for %p\n", __PRETTY_FUNCTION__, name, obj);
            env->ExceptionDescribe();
            env->ExceptionClear();
            fprintf(stderr, "\n");

            env->DeleteLocalRef(cls);
        } else
            fprintf(stderr, "%s: Could not find class for %p\n", __PRETTY_FUNCTION__, obj);
    }

    return 0;
}

template<typename T>
T callJNIMethod(jobject obj, const char* methodName, const char* methodSignature, ...)
{
    va_list args;
    va_start(args, methodSignature);

    T result = callJNIMethodV<T>(obj, methodName, methodSignature, args);

    va_end(args);

    return result;
}

template<typename T>
T callJNIStaticMethod(jclass cls, const char* methodName, const char* methodSignature, ...)
{
    JavaVM* jvm = getJavaVM();
    JNIEnv* env = getJNIEnv();
    va_list args;

    va_start(args, methodSignature);

    T result = 0;

    if (cls && jvm && env) {
        jmethodID mid = env->GetStaticMethodID(cls, methodName, methodSignature);
        if (mid)
            result = JNICaller<T>::callStaticV(cls, mid, args);
        else {
            fprintf(stderr, "%s: Could not find method: %s for %p\n", __PRETTY_FUNCTION__, methodName, cls);
            env->ExceptionDescribe();
            env->ExceptionClear();
            fprintf(stderr, "\n");
        }
    }

    va_end(args);

    return result;
}

} // namespace Bindings

} // namespace JSC

#endif // JNIUtility_h