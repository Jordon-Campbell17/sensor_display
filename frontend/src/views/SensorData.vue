<template>
  <v-container fluid>

    <!-- HEADER STATS CARDS -->
    <v-row class="mb-4">
      <v-col cols="12" md="3" v-for="card in statCards" :key="card.label">
        <v-card elevation="3" rounded="lg">
          <v-card-text class="text-center pa-4">
            <div class="text-caption text-medium-emphasis mb-1">{{ card.label }}</div>
            <div class="text-h4 font-weight-bold" :style="{ color: card.color }">{{ card.value }}</div>
            <div class="text-caption text-medium-emphasis mt-1">{{ card.converted }}</div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- CHARTS ROW -->
    <v-row class="mb-4">
      <!-- Temperature Chart -->
      <v-col cols="12" md="6">
        <v-card elevation="3" rounded="lg">
          <v-card-title class="text-subtitle-1 font-weight-bold pa-4">
            🌡️ Temperature & Heat Index
          </v-card-title>
          <v-card-text>
            <figure class="highcharts-figure">
              <div id="tempChart"></div>
            </figure>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- Humidity Chart -->
      <v-col cols="12" md="6">
        <v-card elevation="3" rounded="lg">
          <v-card-title class="text-subtitle-1 font-weight-bold pa-4">
            💧 Humidity
          </v-card-title>
          <v-card-text>
            <figure class="highcharts-figure">
              <div id="humChart"></div>
            </figure>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <v-row class="mb-4">
      <!-- Pressure Chart -->
      <v-col cols="12" md="6">
        <v-card elevation="3" rounded="lg">
          <v-card-title class="text-subtitle-1 font-weight-bold pa-4">
            🌬️ Atmospheric Pressure
          </v-card-title>
          <v-card-text>
            <figure class="highcharts-figure">
              <div id="pressChart"></div>
            </figure>
          </v-card-text>
        </v-card>
      </v-col>

      <!-- Soil Moisture Chart -->
      <v-col cols="12" md="6">
        <v-card elevation="3" rounded="lg">
          <v-card-title class="text-subtitle-1 font-weight-bold pa-4">
            🌱 Soil Moisture
          </v-card-title>
          <v-card-text>
            <figure class="highcharts-figure">
              <div id="soilChart"></div>
            </figure>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- DATA TABLE -->
    <v-row class="mb-4">
      <v-col cols="12">
        <v-card elevation="3" rounded="lg">
          <v-card-title class="pa-4 d-flex align-center justify-space-between">
            <span class="text-subtitle-1 font-weight-bold">📋 Sensor Data Log</span>
            <v-chip color="success" size="small">
              Auto-refreshing every {{ refreshInterval / 1000 }}s
            </v-chip>
          </v-card-title>
          <v-card-text>
            <v-table density="compact" hover>
              <thead>
                <tr>
                  <th>Timestamp</th>
                  <th>Temp (°C)</th>
                  <th>Temp (°F)</th>
                  <th>Humidity (%)</th>
                  <th>Heat Index (°C)</th>
                  <th>Pressure (hPa)</th>
                  <th>Pressure (mmHg)</th>
                  <th>Soil (%)</th>
                  <th>Soil Status</th>
                </tr>
              </thead>
              <tbody>
                <tr v-for="row in tableData" :key="row.timestamp">
                  <td>{{ formatTime(row.timestamp) }}</td>
                  <td>{{ row.temperature?.toFixed(1) }}</td>
                  <td>{{ toFahrenheit(row.temperature) }}</td>
                  <td>{{ row.humidity?.toFixed(1) }}</td>
                  <td>{{ row.heatindex?.toFixed(1) }}</td>
                  <td>{{ row.pressure?.toFixed(1) }}</td>
                  <td>{{ toMmHg(row.pressure) }}</td>
                  <td>{{ row.soil }}</td>
                  <td>
                    <v-chip :color="soilColor(row.soil)" size="x-small">
                      {{ soilStatus(row.soil) }}
                    </v-chip>
                  </td>
                </tr>
              </tbody>
            </v-table>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

    <!-- CONSOLE LOG -->
    <v-row>
      <v-col cols="12">
        <v-card elevation="3" rounded="lg" color="grey-darken-4">
          <v-card-title class="pa-4 d-flex align-center justify-space-between">
            <span class="text-subtitle-1 font-weight-bold text-white">🖥️ Console Log</span>
            <v-btn size="x-small" variant="outlined" color="white" @click="consoleLogs = []">Clear</v-btn>
          </v-card-title>
          <v-card-text>
            <div class="console-box">
              <div v-for="(log, i) in consoleLogs" :key="i" :class="['console-line', log.type]">
                <span class="console-time">[{{ log.time }}]</span>
                <span class="console-type">[{{ log.type.toUpperCase() }}]</span>
                {{ log.message }}
              </div>
              <div v-if="consoleLogs.length === 0" class="console-line info">
                [INFO] Console ready. Waiting for data...
              </div>
            </div>
          </v-card-text>
        </v-card>
      </v-col>
    </v-row>

  </v-container>
</template>

<script setup>
import { ref, computed, onMounted, onBeforeUnmount } from 'vue'
import Highcharts from 'highcharts'
import more from 'highcharts/highcharts-more'
import Exporting from 'highcharts/modules/exporting'
Exporting(Highcharts)
more(Highcharts)

// ============ STATE ============
const allData       = ref([])
const consoleLogs   = ref([])
const refreshInterval = 10000 // 10 seconds
let   refreshTimer  = null

// ============ CHART OBJECTS ============
const tempChartObj  = ref(null)
const humChartObj   = ref(null)
const pressChartObj = ref(null)
const soilChartObj  = ref(null)

// ============ COMPUTED ============
const latest = computed(() => allData.value.length > 0 ? allData.value[allData.value.length - 1] : null)

const statCards = computed(() => {
  if (!latest.value) return []
  return [
    {
      label: 'Temperature',
      value: `${latest.value.temperature?.toFixed(1)} °C`,
      converted: `${toFahrenheit(latest.value.temperature)} °F`,
      color: tempColor(latest.value.temperature)
    },
    {
      label: 'Humidity',
      value: `${latest.value.humidity?.toFixed(1)} %`,
      converted: latest.value.humidity < 30 ? 'Dry' : latest.value.humidity < 60 ? 'Comfortable' : 'Humid',
      color: '#1E88E5'
    },
    {
      label: 'Pressure',
      value: `${latest.value.pressure?.toFixed(1)} hPa`,
      converted: `${toMmHg(latest.value.pressure)} mmHg`,
      color: '#FB8C00'
    },
    {
      label: 'Soil Moisture',
      value: `${latest.value.soil} %`,
      converted: soilStatus(latest.value.soil),
      color: soilColorHex(latest.value.soil)
    }
  ]
})

const tableData = computed(() => [...allData.value].reverse().slice(0, 20))

// ============ UNIT CONVERSIONS ============
const toFahrenheit = (c) => c != null ? ((c * 9/5) + 32).toFixed(1) : 'N/A'
const toMmHg       = (hpa) => hpa != null && hpa > 0 ? (hpa * 0.750064).toFixed(1) : 'N/A'
const soilStatus   = (s) => {
  if (s < 20) return 'Very Dry'
  if (s < 40) return 'Dry'
  if (s < 70) return 'Moist'
  return 'Wet'
}
const soilColor    = (s) => {
  if (s < 20) return 'error'
  if (s < 40) return 'warning'
  if (s < 70) return 'success'
  return 'info'
}
const soilColorHex = (s) => {
  if (s < 20) return '#E53935'
  if (s < 40) return '#FB8C00'
  if (s < 70) return '#43A047'
  return '#1E88E5'
}
const tempColor    = (t) => {
  if (t < 18) return '#1E88E5'
  if (t < 25) return '#43A047'
  if (t < 30) return '#FB8C00'
  return '#E53935'
}

// ============ FORMATTING ============
const formatTime = (ts) => {
  if (!ts) return 'N/A'
  return new Date(ts * 1000).toLocaleString()
}

// ============ CONSOLE LOG ============
const addLog = (message, type = 'info') => {
  const now = new Date().toLocaleTimeString()
  consoleLogs.value.push({ time: now, message, type })
  if (consoleLogs.value.length > 50) consoleLogs.value.shift()
}

// ============ FETCH DATA ============
const fetchData = async () => {
  addLog('Fetching sensor data from API...', 'info')
  try {
    const response = await fetch('/api/data')
    if (!response.ok) throw new Error(`HTTP ${response.status}`)
    const data = await response.json()
    allData.value = data
    addLog(`Successfully received ${data.length} records from database`, 'success')
    updateCharts(data)
  } catch (err) {
    addLog(`Fetch error: ${err.message}`, 'error')
  }
}

// ============ CREATE CHARTS ============
const createCharts = () => {
  // TEMPERATURE CHART
  tempChartObj.value = Highcharts.chart('tempChart', {
    chart: { zoomType: 'x', height: 250 },
    title: { text: null },
    xAxis: { type: 'datetime', title: { text: 'Time' } },
    yAxis: { title: { text: 'Temperature (°C)' }, labels: { format: '{value} °C' } },
    tooltip: { shared: true },
    series: [
      { name: 'Temperature (°C)', type: 'spline', data: [], color: '#E53935' },
      { name: 'Heat Index (°C)', type: 'spline', data: [], color: '#FB8C00', dashStyle: 'ShortDash' }
    ]
  })

  // HUMIDITY CHART
  humChartObj.value = Highcharts.chart('humChart', {
    chart: { zoomType: 'x', height: 250 },
    title: { text: null },
    xAxis: { type: 'datetime', title: { text: 'Time' } },
    yAxis: { title: { text: 'Humidity (%)' }, labels: { format: '{value} %' }, max: 100 },
    tooltip: { shared: true },
    series: [{ name: 'Humidity (%)', type: 'area', data: [], color: '#1E88E5' }]
  })

  // PRESSURE CHART
  pressChartObj.value = Highcharts.chart('pressChart', {
    chart: { zoomType: 'x', height: 250 },
    title: { text: null },
    xAxis: { type: 'datetime', title: { text: 'Time' } },
    yAxis: { title: { text: 'Pressure (hPa)' }, labels: { format: '{value} hPa' } },
    tooltip: { shared: true },
    series: [{ name: 'Pressure (hPa)', type: 'spline', data: [], color: '#FB8C00' }]
  })

  // SOIL CHART
  soilChartObj.value = Highcharts.chart('soilChart', {
    chart: { zoomType: 'x', height: 250 },
    title: { text: null },
    xAxis: { type: 'datetime', title: { text: 'Time' } },
    yAxis: { title: { text: 'Soil Moisture (%)' }, labels: { format: '{value} %' }, max: 100 },
    tooltip: { shared: true },
    series: [{ name: 'Soil Moisture (%)', type: 'area', data: [], color: '#43A047' }]
  })

  addLog('Charts initialized successfully', 'success')
}

// ============ UPDATE CHARTS ============
const updateCharts = (data) => {
  if (!data || data.length === 0) return

  const tempData    = data.map(d => [d.timestamp * 1000, parseFloat(d.temperature?.toFixed(2)) || 0])
  const heatData    = data.map(d => [d.timestamp * 1000, parseFloat(d.heatindex?.toFixed(2)) || 0])
  const humData     = data.map(d => [d.timestamp * 1000, parseFloat(d.humidity?.toFixed(2)) || 0])
  const pressData   = data.map(d => [d.timestamp * 1000, parseFloat(d.pressure?.toFixed(2)) || 0])
  const soilData    = data.map(d => [d.timestamp * 1000, d.soil || 0])

  if (tempChartObj.value) {
    tempChartObj.value.series[0].setData(tempData)
    tempChartObj.value.series[1].setData(heatData)
  }
  if (humChartObj.value)   humChartObj.value.series[0].setData(humData)
  if (pressChartObj.value) pressChartObj.value.series[0].setData(pressData)
  if (soilChartObj.value)  soilChartObj.value.series[0].setData(soilData)

  addLog('Charts updated with latest data', 'info')
}

// ============ LIFECYCLE ============
onMounted(async () => {
  createCharts()
  await fetchData()
  refreshTimer = setInterval(fetchData, refreshInterval)
  addLog(`Auto-refresh enabled every ${refreshInterval / 1000} seconds`, 'info')
})

onBeforeUnmount(() => {
  clearInterval(refreshTimer)
  addLog('Page unmounted. Auto-refresh stopped.', 'info')
})
</script>

<style scoped>
.highcharts-figure { margin: 0; }
.console-box {
  background: #1a1a1a;
  border-radius: 8px;
  padding: 12px;
  height: 200px;
  overflow-y: auto;
  font-family: 'Courier New', monospace;
  font-size: 12px;
}
.console-line { margin-bottom: 4px; }
.console-line.info    { color: #64B5F6; }
.console-line.success { color: #81C784; }
.console-line.error   { color: #E57373; }
.console-line.warning { color: #FFB74D; }
.console-time { color: #888; margin-right: 6px; }
.console-type { margin-right: 6px; font-weight: bold; }
</style>