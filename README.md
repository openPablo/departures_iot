# departures_iot

## Building reqs
```
sudo dnf install curl-devel
```

## Compiling

```
make
```

## Running
Pass the DELIJN api key, and a comma seperated list of haltes.
Get haltes IDs from here: [link](https://data.delijn.be/api-details#api=KernOpenDataServicesV1&operation=get-haltes-indebuurt-latlng)
```
export DELIJN_API_KEY="--------------"
./departures_iot
```
