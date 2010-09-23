/*
 * Copyright (C) 2010 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.i18n.addressinput;

/**
 * A helper class to let the calling thread wait until loading has finished.
 */
public class NotifyingListener implements DataLoadListener {
    private Object mSleeper;
    private boolean mDone;

    public NotifyingListener(Object sleeper) {
        mSleeper = sleeper;
        mDone = false;
    }

    public void dataLoadingBegin() {
    }

    public void dataLoadingEnd() {
        synchronized (this) {
            mDone = true;
        }
        synchronized (mSleeper) {
            mSleeper.notify();
        }
    }

    public void waitLoadingEnd() throws InterruptedException {
        synchronized (this) {
            if (mDone) return;
        }
        synchronized (mSleeper) {
            mSleeper.wait();
        }
    }
}
