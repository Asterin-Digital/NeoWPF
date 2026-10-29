package com.asterin.neowpf.android

import android.app.Activity
import android.os.Bundle
import android.widget.TextView

class MainActivity : Activity() {
    external fun nativeStartup(): String

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        System.loadLibrary("neowpf_android")
        val status = nativeStartup()
        setContentView(TextView(this).apply {
            text = status
            textSize = 18f
        })
    }
}