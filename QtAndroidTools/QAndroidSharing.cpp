/*
 *	MIT License
 *
 *	Copyright (c) 2018 Fabio Falsini <falsinsoft@gmail.com>
 *
 *	Permission is hereby granted, free of charge, to any person obtaining a copy
 *	of this software and associated documentation files (the "Software"), to deal
 *	in the Software without restriction, including without limitation the rights
 *	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *	copies of the Software, and to permit persons to whom the Software is
 *	furnished to do so, subject to the following conditions:
 *
 *	The above copyright notice and this permission notice shall be included in all
 *	copies or substantial portions of the Software.
 *
 *	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *	SOFTWARE.
 */
#include "QAndroidSharing.h"

QAndroidSharing *QAndroidSharing::m_pInstance = nullptr;

QAndroidSharing::QAndroidSharing() : m_JavaSharing("com/falsinsoft/qtandroidtools/AndroidSharing",
                                                   "(Landroid/app/Activity;)V",
                                                   QtAndroid::androidActivity().object<jobject>())
{
    m_pInstance = this;

    if(m_JavaSharing.isValid())
    {
        const JNINativeMethod JniMethod[] = {
            {"sharedDataReceived", "(Landroid/content/Intent;)V", reinterpret_cast<void *>(&QAndroidSharing::SharedDataReceived)}
        };
        QAndroidJniEnvironment JniEnv;
        jclass ObjectClass;

        ObjectClass = JniEnv->GetObjectClass(m_JavaSharing.object<jobject>());
        JniEnv->RegisterNatives(ObjectClass, JniMethod, sizeof(JniMethod)/sizeof(JNINativeMethod));
        JniEnv->DeleteLocalRef(ObjectClass);
    }
}

QObject* QAndroidSharing::qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    return new QAndroidSharing();
}

QAndroidSharing* QAndroidSharing::instance()
{
    return m_pInstance;
}

void QAndroidSharing::shareText(const QString &Text)
{
    if(m_JavaSharing.isValid())
    {
        m_JavaSharing.callMethod<void>("shareText",
                                       "(Ljava/lang/String;)V",
                                       QAndroidJniObject::fromString(Text).object<jstring>()
                                       );
    }
}

void QAndroidSharing::ProcessDataReceived(const QAndroidJniObject &DataIntent)
{
    const char IntentClass[] = "android/content/Intent";

    const QString ActionSendId = QAndroidJniObject::getStaticObjectField<jstring>(IntentClass, "ACTION_SEND").toString();
    const QString ExtraTextId = QAndroidJniObject::getStaticObjectField<jstring>(IntentClass, "EXTRA_TEXT").toString();

    if(DataIntent.isValid())
    {
        const QString Action = DataIntent.callObjectMethod("getAction", "()Ljava/lang/String;").toString();
        const QString Type = DataIntent.callObjectMethod("getType", "()Ljava/lang/String;").toString();

        if(Action == ActionSendId)
        {
            if(Type == "text/plain")
            {
                const QString SharedText = DataIntent.callObjectMethod("getStringExtra",
                                                                       "(Ljava/lang/String;)Ljava/lang/String;",
                                                                       QAndroidJniObject::fromString(ExtraTextId).object<jstring>()
                                                                       ).toString();
                emit sharedTextReceived(SharedText);
            }
        }
    }
}

void QAndroidSharing::SharedDataReceived(JNIEnv *env, jobject thiz, jobject dataIntent)
{
    Q_UNUSED(env)
    Q_UNUSED(thiz)

    if(m_pInstance != nullptr)
    {
        m_pInstance->ProcessDataReceived(QAndroidJniObject(dataIntent));
    }
}
