
package com.caredear.backend.rs;

/**
 * @ClassName: RequestPayload
 * @Description: init payload(Desp what does this class do)
 * @author HeQi heqi@caredear.com
 * @date Jun 29, 2014 10:38:45 PM
 */
public class RequestPayload {

    private String payLoad;

    /**
     * @Title: getPayload
     * @Description: payload = userName + SPLIT + appName + SPLIT +
     *               Long.toString(time) + SPLIT + token;(Desc what does this
     *               func do)
     * @param @return Setting file
     * @return String return type
     * @throws
     */
    public RequestPayload(String payload) {
        this.payLoad = payload;
    }

    public String getPayload() {

        String header = Integer.toHexString(payLoad.length());
        int body = header.length();
        for (int i = 4; i - body > 0; i--) {
            header = "0" + header;
        }
        return header + payLoad;
    }
}
