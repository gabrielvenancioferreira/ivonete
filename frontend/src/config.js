const params = new URLSearchParams(window.location.search);
const apiFromQuery = params.get("api");

export const API_BASE_URL = apiFromQuery || localStorage.getItem("apiBaseUrl") || "http://localhost:8080";
export const REQUEST_TIMEOUT_MS = 10000;
