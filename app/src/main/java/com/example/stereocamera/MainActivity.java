package com.example.stereocamera;

import androidx.appcompat.app.AppCompatActivity;
import androidx.annotation.NonNull;
import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;
import com.example.stereocamera.databinding.ActivityMainBinding;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("opencv_java4");
        System.loadLibrary("stereo_camera");
    }

    private ActivityMainBinding binding;
    private Bitmap cameraBitmap;
    private Handler renderHandler = new Handler(Looper.getMainLooper());
    private boolean isRunning = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        // Reduced resolution to 320x240 for stability during testing
        cameraBitmap = Bitmap.createBitmap(320, 240, Bitmap.Config.ARGB_8888);

        if (checkSelfPermission(Manifest.permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(new String[]{Manifest.permission.CAMERA}, 101);
        } else {
            // Added 500ms delay to ensure camera service is ready
            renderHandler.postDelayed(this::startSingleCamera, 500);
        }
    }

    private void startSingleCamera() {
        // Try opening ONLY camera "0"
        // Note: We use the existing native signature but pass the same ID twice or null for ID2
        if (initializeCameras("1", "1")) {
            isRunning = true;
            renderLoop.run();
        } else {
            Toast.makeText(this, "Failed to open camera 0.", Toast.LENGTH_LONG).show();
        }
    }

    private final Runnable renderLoop = new Runnable() {
        @Override
        public void run() {
            if (!isRunning) return;

            // This will now just copy camera 0 frames to the bitmap
            getSingleFrame(cameraBitmap);

            binding.disparityView.setImageBitmap(cameraBitmap);
            renderHandler.postDelayed(this, 33);
        }
    };

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 101 && grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            renderHandler.postDelayed(this::startSingleCamera, 500);
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        isRunning = false;
        releaseCameras();
    }

    // Native methods
    public native boolean initializeCameras(String idL, String idR);
    public native void getSingleFrame(Bitmap bitmap); // Renamed for clarity in testing
    public native void releaseCameras();
}
