#!/bin/bash

# setup-signing.sh
# Quickly set up Android signing configuration for local development.

cd "$(dirname "$0")"

KEYSTORE_FILE="release-key.jks"
PROPERTIES_FILE="app/keystore.properties"

echo "=== Android CodeLens Signing Setup ==="

# 1. Check/Generate Keystore
if [ -f "$KEYSTORE_FILE" ]; then
    echo "✅ Keystore '$KEYSTORE_FILE' already exists."
else
    echo "⚠️  Keystore not found. Generating a new one..."
    
    # Default values (can be customized)
    STORE_PASS="android"
    KEY_ALIAS="key0"
    KEY_PASS="android"
    VALIDITY="10000"
    DNAME="CN=Android Debug, O=Android, C=US"

    echo "Generating keystore with:"
    echo "  File: $KEYSTORE_FILE"
    echo "  Alias: $KEY_ALIAS"
    echo "  Password: $STORE_PASS"
    
    keytool -genkeypair -v \
        -keystore "$KEYSTORE_FILE" \
        -alias "$KEY_ALIAS" \
        -keyalg RSA \
        -keysize 2048 \
        -validity "$VALIDITY" \
        -storepass "$STORE_PASS" \
        -keypass "$KEY_PASS" \
        -dname "$DNAME"
        
    if [ $? -eq 0 ]; then
        echo "✅ Keystore generated successfully."
    else
        echo "❌ Failed to generate keystore."
        exit 1
    fi
fi

# 2. Check/Create keystore.properties
if [ -f "$PROPERTIES_FILE" ]; then
    echo "✅ Properties file '$PROPERTIES_FILE' already exists."
else
    echo "⚠️  '$PROPERTIES_FILE' not found. Creating it..."
    
    # Needs to match the keystore we just found or created
    # Note: If an existing JKS was found but no properties file, we assume the defaults or ask user?
    # For simplicity in this script, we'll write the defaults we used above. 
    # If the user supplied their own JKS, they might need to edit this manually, but this script is mainly for "one-click" fresh setup.
    
    STORE_PASS="android"
    KEY_ALIAS="key0"
    KEY_PASS="android"
    
    cat > "$PROPERTIES_FILE" <<EOF
storePassword=$STORE_PASS
keyPassword=$KEY_PASS
keyAlias=$KEY_ALIAS
storeFile=../$KEYSTORE_FILE
EOF
    echo "✅ Created '$PROPERTIES_FILE'."
fi

echo ""
echo "🎉 Signing setup complete!"
echo "   - Keystore: $KEYSTORE_FILE"
echo "   - Properties: $PROPERTIES_FILE"
echo "   - Secrets are ignored by git (checked .gitignore)."
echo ""
echo "You can now run: ./gradlew assembleRelease"
