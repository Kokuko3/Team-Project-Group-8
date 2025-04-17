package com.example.flaskimageviewer

import android.os.Bundle
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.text.BasicTextField
import androidx.compose.material.*
import androidx.compose.runtime.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
import androidx.compose.ui.viewinterop.AndroidView
import android.widget.Toast
import android.webkit.WebSettings

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            FlaskImageViewerApp()
        }
    }
}

@Composable
fun FlaskImageViewerApp() {
    var ipAddress by remember { mutableStateOf("") }
    var url by remember { mutableStateOf("") }
    val context = LocalContext.current
    var reloadTrigger by remember { mutableStateOf(false) } // Used to trigger reload
    var webView: WebView? by remember { mutableStateOf(null) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(16.dp)
    ) {
        OutlinedTextField(
            value = ipAddress,
            onValueChange = { ipAddress = it },
            label = { Text("Enter Flask Server IP Address") },
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(modifier = Modifier.height(8.dp))

        Button(onClick = {
            if (ipAddress.isNotBlank()) {
                url = "http://$ipAddress"
                reloadTrigger = !reloadTrigger // Just to reset and show updated site
            } else {
                Toast.makeText(context, "Please enter an IP address", Toast.LENGTH_SHORT).show()
            }
        }) {
            Text("Load Website")
        }

        Spacer(modifier = Modifier.height(8.dp))

        if (url.isNotBlank()) {
            Button(onClick = {
                webView?.reload()
            }) {
                Text("Reload Website")
            }

            Spacer(modifier = Modifier.height(16.dp))

            WebViewScreen(url, webViewSetter = { webView = it }, reloadTrigger)
        }
    }
}


@Composable
fun WebViewScreen(
    url: String,
    webViewSetter: (WebView) -> Unit,
    reloadTrigger: Boolean
) {
    AndroidView(
        factory = { ctx ->
            WebView(ctx).apply {
                settings.javaScriptEnabled = true
                settings.cacheMode = WebSettings.LOAD_NO_CACHE
                webViewClient = WebViewClient()
                loadUrl(url)
                webViewSetter(this) // Pass WebView back to Main
            }
        },
        update = {
            it.loadUrl(url) // Reload when trigger changes
        },
        modifier = Modifier.fillMaxSize()
    )
}
