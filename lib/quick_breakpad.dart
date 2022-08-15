
import 'dart:async';

import 'package:flutter/services.dart';

class QuickBreakpad {
  static const MethodChannel _channel =
      const MethodChannel('quick_breakpad');

  static Future<String> get platformVersion async {
    final String version = await _channel.invokeMethod('getPlatformVersion');
    return version;
  }

  static Future<void> regCrashHandler(String path) async {
    await _channel.invokeMethod('regCrashHandler', path);
  }
}
