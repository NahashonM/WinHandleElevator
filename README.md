# WOBDriver

## installing

This driver is not signed and cannot be loaded without turning on `testsigning`.

```sc create WOBDriver binPath=<path_to_driver> type=kernel start=demand error=normal```


## starting

```sc start WOBDriver```


## stopping

```sc stop WOBDriver```


## Quering current status

```sc query WOBDriver```


## uninstalling

```sc delete WOBDriver```


# IOCTLS

- IOCTL_OPEN_PROCESS_METHOD_BUFFERED

  Exposes kernel mode handles to user mode process.
  
  Note: This is the intendend behaviour for  this driver.

- IOCTL_ELEVATE_HANDLE_METHOD_BUFFERED
  
