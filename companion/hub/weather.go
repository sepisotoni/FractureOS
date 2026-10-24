package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"time"
)

type WeatherData struct {
	TempTenths int16
	Humidity   uint8
	Code       uint8
	HighTenths int16
	LowTenths  int16
	LastUpdate time.Time
}

type openMeteoResponse struct {
	Current struct {
		Temperature float64 `json:"temperature_2m"`
		Humidity    float64 `json:"relative_humidity_2m"`
		WeatherCode int     `json:"weather_code"`
	} `json:"current"`
	Daily struct {
		MaxTemp []float64 `json:"temperature_2m_max"`
		MinTemp []float64 `json:"temperature_2m_min"`
	} `json:"daily"`
}

var cachedWeather = WeatherData{
	TempTenths: 215, // 21.5 C default
	Humidity:   55,
	Code:       0,
	HighTenths: 260,
	LowTenths:  140,
	LastUpdate: time.Now(),
}

// FetchWeather queries Open-Meteo API without requiring any API keys.
// Falls back to cached data if network or API is unavailable.
func FetchWeather(lat, lon float64) WeatherData {
	if time.Since(cachedWeather.LastUpdate) < 30*time.Minute && cachedWeather.TempTenths != 0 {
		return cachedWeather
	}

	url := fmt.Sprintf(
		"https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m,weather_code&daily=temperature_2m_max,temperature_2m_min&timezone=auto",
		lat, lon,
	)

	client := &http.Client{Timeout: 5 * time.Second}
	resp, err := client.Get(url)
	if err != nil {
		return cachedWeather
	}
	defer resp.Body.Close()

	if resp.StatusCode != http.StatusOK {
		return cachedWeather
	}

	var data openMeteoResponse
	if err := json.NewDecoder(resp.Body).Decode(&data); err != nil {
		return cachedWeather
	}

	cachedWeather.TempTenths = int16(data.Current.Temperature * 10)
	cachedWeather.Humidity = uint8(data.Current.Humidity)
	cachedWeather.Code = uint8(data.Current.WeatherCode)
	if len(data.Daily.MaxTemp) > 0 {
		cachedWeather.HighTenths = int16(data.Daily.MaxTemp[0] * 10)
	}
	if len(data.Daily.MinTemp) > 0 {
		cachedWeather.LowTenths = int16(data.Daily.MinTemp[0] * 10)
	}
	cachedWeather.LastUpdate = time.Now()

	return cachedWeather
}
