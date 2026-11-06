# Network configuration (binding)

Both `DBSrv` and `TMSrv` read the address they bind to from `localip.txt` (looked up in the run directory first, then `../../Common/localip.txt`).

* `127.0.0.1` or `localhost` - only accept connections from the local machine (default when the file is missing/invalid).
* `0.0.0.0`, `any`, `all` or `*` - listen on every interface (LAN/WAN).
* A specific IPv4 such as `192.168.1.10` - listen only on that interface.

`TMSrv` uses the same value to find its server group in `serverlist.txt` and to locate `DBSrv` (entry `[group][0]`). When the bind address is loopback (or `0.0.0.0`), a matching entry is not required: it falls back to `127.0.0.1` (loopback) or the first group's DB address (`0.0.0.0`).

To force a different DB address/port, create `dbserver.txt` in the `TMSrv` run directory:

```
127.0.0.1 7514
```

`DBSrv` accepts a `TMSrv` connecting from loopback without an entry in `serverlist.txt`, which makes local testing work out of the box. For LAN/WAN play, set `localip.txt` to the public/LAN address and keep that address in `serverlist.txt`.
