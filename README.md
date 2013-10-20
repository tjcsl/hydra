hydra
=====

Cluster job management software.

Usage
=====

Edit `/etc/hydra/master.conf` on the server that will be hosting `hydramd` to
whitelist your nodes. These can be FQDNs or IP addresses. Run `hydramd` on the
master, either manually or using the provided OpenRC initscript.

Edit `/etc/hydra/slave.conf` on your nodes to include the FQDN or IP address of
the master. Run `hydrasd` on the slaves manually or using the provided OpenRC
initscript.

Distribute `hydrarun` to your users. Usage is 
```
hydrarun [-d <data> [-d <data> ...]] -s NUM -e <executable> -- [args]
    -d  		    | Specifies a file to be uploaded and placed in the same 
                    |       directory as the executable on the slaves.
    -s              | Specifies the number of slots to request. Essentialy
                    |       translates to number of CPU cores that will be used to
                    |       run your program.
    -e <executable> | The first argument that does not match -d or -s (or their long
                    |       versions) is assumed to be the name of the executable to
                    |       run on the cluster
    [args]          | Any additional arguments are forwarded to your application


```
