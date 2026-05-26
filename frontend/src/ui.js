import { CATEGORIES } from "./state.js";

const moneyFormatter = new Intl.NumberFormat("pt-BR", {
  style: "currency",
  currency: "BRL"
});

const percentFormatter = new Intl.NumberFormat("pt-BR", {
  minimumFractionDigits: 1,
  maximumFractionDigits: 1
});

export function getElements() {
  return {
    apiBaseUrl: document.getElementById("api-base-url"),
    feedback: document.getElementById("feedback"),
    userForm: document.getElementById("user-form"),
    userName: document.getElementById("user-name"),
    usersContainer: document.getElementById("users-container"),
    refreshUsersButton: document.getElementById("refresh-users"),
    plannerForm: document.getElementById("planner-form"),
    investorName: document.getElementById("investor-name"),
    monthlyIncome: document.getElementById("monthly-income"),
    idealPlan: document.getElementById("ideal-plan"),
    weightsForm: document.getElementById("weights-form"),
    weightsTotal: document.getElementById("weights-total"),
    currentForm: document.getElementById("current-form"),
    summary: document.getElementById("summary"),
    rebalanceButton: document.getElementById("rebalance-btn")
  };
}

export function setApiBaseUrl(element, baseUrl) {
  element.textContent = baseUrl;
}

export function setFeedback(element, message, type = "neutral") {
  element.className = `feedback ${type}`;
  element.textContent = message;
}

export function renderUsers(container, users, onDelete) {
  if (!users.length) {
    container.innerHTML = "<p class='hint'>Nenhum usuario encontrado no backend.</p>";
    return;
  }

  const list = document.createElement("ul");
  list.className = "users-list";

  users.forEach((user) => {
    const item = document.createElement("li");
    const text = document.createElement("span");
    text.textContent = `#${user.id ?? "?"} - ${user.nome ?? "Sem nome"}`;

    const button = document.createElement("button");
    button.type = "button";
    button.className = "btn btn-danger";
    button.textContent = "Excluir";
    button.addEventListener("click", () => onDelete(user.id));

    item.appendChild(text);
    item.appendChild(button);
    list.appendChild(item);
  });

  container.innerHTML = "";
  container.appendChild(list);
}

export function renderIdealPlan(element, plan, monthlyIncome) {
  if (!plan) {
    element.innerHTML = "<p class='hint'>Informe o ganho mensal para gerar o plano.</p>";
    return;
  }

  const weightLines = CATEGORIES.map(
    (category) =>
      `<li>${category.label}: <strong>${percentFormatter.format(plan.suggestedWeights[category.key])}%</strong></li>`
  ).join("");

  element.innerHTML = `
    <p><strong>Perfil sugerido:</strong> ${plan.profile}</p>
    <p><strong>Aporte mensal recomendado:</strong> ${moneyFormatter.format(plan.monthlyContribution)} (${percentFormatter.format(
      plan.investRate * 100
    )}% de ${moneyFormatter.format(monthlyIncome)})</p>
    <p><strong>Pesos sugeridos:</strong></p>
    <ul>${weightLines}</ul>
  `;
}

export function renderWeightsHint(element, total) {
  const diff = Math.abs(total - 100);
  if (diff < 0.0001) {
    element.textContent = "Pesos validos: soma exata de 100%.";
    element.style.color = "#22c55e";
  } else {
    element.textContent = `A soma atual e ${percentFormatter.format(total)}%. Ajuste para fechar em 100%.`;
    element.style.color = "#fca5a5";
  }
}

export function renderSummary(element, result) {
  if (!result) {
    element.innerHTML = "<p class='hint'>Clique em calcular para ver o rebalanceamento.</p>";
    return;
  }

  const rows = result.rows
    .map((row) => {
      const actionText =
        row.delta >= 0
          ? `Investir ${moneyFormatter.format(row.delta)}`
          : `Reduzir ${moneyFormatter.format(Math.abs(row.delta))}`;

      return `
        <tr>
          <td>${row.label}</td>
          <td>${moneyFormatter.format(row.currentValue)}</td>
          <td>${percentFormatter.format(row.targetPercent)}%</td>
          <td>${moneyFormatter.format(row.targetValue)}</td>
          <td>${actionText}</td>
        </tr>
      `;
    })
    .join("");

  element.innerHTML = `
    <p><strong>Total atual:</strong> ${moneyFormatter.format(result.currentTotal)}</p>
    <p><strong>Aporte planejado:</strong> ${moneyFormatter.format(result.plannedContribution)}</p>
    <p><strong>Total projetado:</strong> ${moneyFormatter.format(result.projectedTotal)}</p>
    <p><strong>Total a investir para equilibrar:</strong> ${moneyFormatter.format(result.totalPositiveDelta)}</p>
    <table>
      <thead>
        <tr>
          <th>Classe</th>
          <th>Atual</th>
          <th>Peso alvo</th>
          <th>Valor alvo</th>
          <th>Acao</th>
        </tr>
      </thead>
      <tbody>${rows}</tbody>
    </table>
  `;
}
