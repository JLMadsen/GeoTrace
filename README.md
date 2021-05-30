# 🌍 TraceRouter (Work in progress 👷‍♂️)
Plots a traceroute path on a map. 

<p align="center">
  
  <img src="https://i.imgur.com/CmsYPLm.gif"/>
  
</p>

Made with <a href="https://www.qt.io/">Qt</a>!

## Usage

Currently using my private geolocation API key, get your own here: https://ipgeolocation.io/

Create an enviornment variable `tr_api_key` with the API key as value.

```c++
geolocation_api_key = QProcessEnvironment::systemEnvironment().value("tr_api_key");
```

## About

This program was made in the span of a week as one of my side projects during the summer.

Watching the development of <a href="https://github.com/SerenityOS/serenity">SerenityOS</a> made me want to give C++ another try.

## Todo

* Make GUI elements responsive.
* Fix random crashes.
