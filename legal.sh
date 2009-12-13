echo "Doing insane legal stuff the lawyers insist we have to do"
find . -type f -exec sed -i 's/\bscanners\b/acquisition devices/g' {} \;
find . -type f -exec sed -i 's/\bScanners\b/Acquisition devices/g' {} \;
find . -type f -exec sed -i 's/\bscanner\b/acquisition device/g' {} \;
find . -type f -exec sed -i 's/\bScanner\b/Acquisition device/g' {} \;
find . -type f -exec sed -i 's/\bscanning\b/acquiring/g' {} \;
find . -type f -exec sed -i 's/\bScanning\b/Acquiring/g' {} \;
find . -type f -exec sed -i 's/\bscan\b/acquisition/g' {} \;
find . -type f -exec sed -i 's/\bScan\b/Acquisition/g' {} \;
find . -type f -exec sed -i 's/\bscanned\b/acquired/g' {} \;
find . -type f -exec sed -i 's/\bScanned\b/Acquired/g' {} \;

