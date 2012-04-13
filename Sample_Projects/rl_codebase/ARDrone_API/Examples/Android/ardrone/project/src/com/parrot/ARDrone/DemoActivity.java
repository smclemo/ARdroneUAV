/*
 * AR Drone demo
 * This file is based on the NDK sample app "San Angeles"
 */
package com.parrot.ARDrone;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.WindowManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.util.Log;
import android.view.KeyEvent;

public class DemoActivity extends Activity implements SensorEventListener {

	private SensorManager mSensorManager;
    private GLSurfaceView mGLView;
	private Sensor mSensor;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mGLView = new DemoGLSurfaceView(this);
        setContentView(mGLView);

        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
                             WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
		// sensor events
		mSensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
		mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ORIENTATION);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
		mSensorManager.unregisterListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
		// update every 200 ms (NORMAL), 60 ms (UI) or 20 ms (GAME)
        mSensorManager.registerListener(this, mSensor, SensorManager.SENSOR_DELAY_GAME);
    }

    @Override
    protected void onStop() {
        super.onStop();
        nativeStop();
    }

	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		/* do nothing ? */
    }

    public void onSensorChanged(final SensorEvent ev) {
		nativeSensorEvent(ev.values[0],ev.values[1],ev.values[2]);
    }

    static {
        System.loadLibrary("ardrone");
    }
    private static native void nativeStop();
	private static native void nativeSensorEvent(float x, float y, float z);
}

class DemoGLSurfaceView extends GLSurfaceView {
	private DemoRenderer mRenderer;
	private static final String TAG = "ARDrone";

	public DemoGLSurfaceView(Context context) {
        super(context);
        mRenderer = new DemoRenderer();
        setRenderer(mRenderer);

        // receive events
        setFocusable(true);
        setFocusableInTouchMode(true);
        requestFocus();
    }

    @Override
    public boolean onTrackballEvent(final MotionEvent ev) {
        queueEvent(new Runnable() {
                public void run() {
                    nativeTrackballEvent(ev.getEventTime(),
                                         ev.getAction(),
                                         ev.getX(),
										 ev.getY());
                }
            });
        return true;
    }

    @Override
    public boolean dispatchTouchEvent(final MotionEvent ev) {
        queueEvent(new Runnable() {
                public void run() {
					//Log.v(TAG, "event=" + ev);
                    nativeMotionEvent(ev.getEventTime(),
                                      ev.getAction(),
                                      ev.getX(),
									  ev.getY());
				}
            });
        return true;
    }

    private static native void nativePause();
    private static native void nativeMotionEvent(long eventTime, int action,
												 float x, float y);
	private static native void nativeTrackballEvent(long eventTime,
													int action,
													float x, float y);
	//private static native void nativeKeyEvent(int action);
}

class DemoRenderer implements GLSurfaceView.Renderer {
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit();
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        //gl.glViewport(0, 0, w, h);
        nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) {
        nativeRender();
    }

    private static native void nativeInit();
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();
    //private static native void nativeDone();
}
