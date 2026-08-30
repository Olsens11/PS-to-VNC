# M1D Hardware Regression

## Result

M1D real-hardware validation is complete.

Exact M1 DUT:

    SHA256: 26ae06ff72226b0542e26865b195132c7c6eefba2e5beabc2cdd343ae5c8105b
    Bytes:  2927748

Machine result:

    PASS

Physical result:

    PASS

Startup gate:

    PASS

Evidence:

    evidence/m1/m1d2-hardware-20260830-022035/

Evidence manifest SHA256:

    f63ce1682751012a10103f679475f6f25da18bcd0ad83c78a02668d1fc72c53c

## Startup-control resolution

The first M1 launch encountered a common Pi-side management-service failure.

The exact historical M0/B4A control then encountered the same common startup
condition. That initial symptom therefore did not demonstrate an M1
regression.

After targeted recovery of the Pi management service:

- exact M0/B4A reached the normal desktop;
- exact M1 reached the normal desktop.

The M1 startup gate passed.

## Required five-mode hardware regression

The exact M1 DUT completed the required hands-off sequence across:

1. 480i
2. 480p-hires
3. 576i
4. 720p
5. 1080i

Machine authority:

- harness return code 0;
- PASS_COUNT=5;
- FAIL_COUNT=0;
- final txid=0;
- final state=IDLE;
- startup_mode=480p;
- automatic failure recovery disabled.

Independent physical authority:

- operator result PASS;
- all five requested modes visibly appeared;
- every rollback returned normally to 480p;
- cursor behavior returned normally;
- no freeze, green/corrupt output, or persistent no-signal was observed.

## Migration conclusion

The first mechanical translation-unit extraction passed its required
real-hardware regression.

M1D hardware authority is complete.

The next migration operation is M1E: close M1 and select the next mechanical
extraction boundary.
