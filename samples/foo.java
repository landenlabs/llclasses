
    @Override
    public void writeToParcel(Parcel dest, int flags) {
        super.writeToParcel(dest, flags);
        dest.writeList(alerts);
    }

    @SuppressWarnings("unchecked")
    private WxAlertHeadlines(Parcel source) {
        super.readFromParcel(source);
        alerts = source.readArrayList(Alert.class.getClassLoader());
    }


    // =============================================================================================
    public static class Alert implements Comparable<Alert>, Parcelable {
        public String detailKey;                   // "59c65e0f-9c3b-3daf-b0b0-f804e56d6e87"
