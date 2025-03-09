import 'dart:async';
import 'dart:convert';
import 'package:flutter/material.dart';
import 'package:http/http.dart' as http;

void main() {
  runApp(MyApp());
}

class MyApp extends StatefulWidget {
  @override
  _MyAppState createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  double temperature = 0.0;
  double humidity = 0.0;
  double voltage = 0.0;
  double current = 0.0;

  final String esp32Url = "http://192.168.231.201/";

  @override
  void initState() {
    super.initState();
    fetchData();
    Timer.periodic(Duration(seconds: 2), (timer) => fetchData());
  }

  Future<void> fetchData() async {
    try {
      final response = await http.get(Uri.parse(esp32Url));
      if (response.statusCode == 200) {
        final data = json.decode(response.body);
        setState(() {
          temperature = data['temperature'];
          humidity = data['humidity'];
          voltage = data['voltage'];
          current = data['current'];
        });
      } else {
        print("ESP32'den veri alınamadı!");
      }
    } catch (e) {
      print("Bağlantı hatası: $e");
    }
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(title: Text('Solar Panel Monitor')),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.center,
            children: [
              Text('Temperature: $temperature °C', style: TextStyle(fontSize: 18)),
              Text('Humidity: $humidity %', style: TextStyle(fontSize: 18)),
              Text('Voltage: $voltage V', style: TextStyle(fontSize: 18)),
              Text('Current: $current A', style: TextStyle(fontSize: 18)),
            ],
          ),
        ),
      ),
    );
  }
}
