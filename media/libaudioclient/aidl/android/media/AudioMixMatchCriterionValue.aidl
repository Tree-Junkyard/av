/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package android.media;

import android.media.audio.common.AudioSource;
import android.media.audio.common.AudioUsage;

/**
 * {@hide}
 */
union AudioMixMatchCriterionValue {
    AudioUsage usage = AudioUsage.UNKNOWN;
    AudioSource source;
    /** Interpreted as uid_t. */
    int uid;
    int userId;
    /** Interpreted as audio_session_t. */
    int audioSessionId;
}
