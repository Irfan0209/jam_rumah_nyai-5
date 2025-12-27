void kirimDisplay() {
  Serial.print("DISPLAY:");
  Serial.print("Br=");    Serial.print(brightness);
  Serial.print(",Sptx1=");Serial.print(speedText1);
  Serial.print(",Sptx2=");Serial.print(speedText2);
  Serial.print(",Spdt="); Serial.print(speedDate);
  Serial.print(",Spnm="); Serial.println(speedName);
}


void kirimLokasi() {
  Serial.print("LOKASI:");
  Serial.print("Lt="); Serial.print(config.latitude, 6);
  Serial.print(",Lo="); Serial.print(config.longitude, 6);
  Serial.print(",Tm="); Serial.print(config.zonawaktu);
  Serial.print(",Al="); Serial.println(config.altitude);
}


void kirimPesan() {
  Serial.print("PESAN:");
  Serial.print("m1=");   Serial.print(text1);
  Serial.print(",m2=");  Serial.print(text2);
  Serial.print(",name=");Serial.print(name);
  Serial.print(",idx="); Serial.println(counterName);
}


void kirimKoreksi() {
  Serial.print("KOREKSI:");
  for (int i = 0; i < 5; i++) {
    Serial.print(i + 1);
    Serial.print("-");
    Serial.print(dataIhty[i]);
    if (i < 4) Serial.print(",");
  }
  Serial.println();
}


void kirimIqomah() {
  Serial.print("IQOMAH:");

  for (int i = 0; i < 5; i++) {
    Serial.print("I");
    Serial.print(i + 1);
    Serial.print("-");
    Serial.print(iqomah[i]);
    Serial.print(",");
  }

  for (int i = 0; i < 5; i++) {
    Serial.print("B");
    Serial.print(i + 1);
    Serial.print("-");
    Serial.print(displayBlink[i]);
    if (i < 4) Serial.print(",");
  }
  Serial.println();
}
