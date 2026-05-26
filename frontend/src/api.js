import { REQUEST_TIMEOUT_MS } from "./config.js";

export class ApiClient {
  constructor(baseUrl) {
    this.baseUrl = baseUrl.replace(/\/+$/, "");
  }

  async request(path, options = {}) {
    const controller = new AbortController();
    const timeoutId = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);
    const url = `${this.baseUrl}${path}`;

    const headers = {
      Accept: "application/json",
      "Content-Type": "application/json",
      ...(options.headers || {})
    };

    try {
      const response = await fetch(url, {
        ...options,
        headers,
        signal: controller.signal
      });

      clearTimeout(timeoutId);
      const contentType = response.headers.get("content-type") || "";
      const hasJson = contentType.includes("application/json");
      const payload = hasJson ? await response.json() : await response.text();

      if (!response.ok) {
        const messageFromBody =
          payload && typeof payload === "object" && payload.erro
            ? payload.erro
            : `Servidor respondeu com status ${response.status}.`;
        throw new Error(messageFromBody);
      }

      return payload;
    } catch (error) {
      clearTimeout(timeoutId);
      if (error.name === "AbortError") {
        throw new Error("Tempo de resposta excedido. Tente novamente em alguns segundos.");
      }
      if (error instanceof TypeError) {
        throw new Error(
          "Nao foi possivel conectar ao backend C. Confirme se a API esta ativa e se o CORS esta habilitado."
        );
      }
      throw error;
    }
  }

  async getUsers() {
    const data = await this.request("/usuarios", { method: "GET" });
    return Array.isArray(data) ? data : [];
  }

  async createUser(user) {
    return this.request("/usuarios", {
      method: "POST",
      body: JSON.stringify(user)
    });
  }

  async deleteUser(id) {
    return this.request(`/usuarios/${id}`, { method: "DELETE" });
  }
}
