import { API_BASE_URL } from "./config.js";
import { ApiClient } from "./api.js";
import {
  state,
  defaultWeights,
  parseAllocationForm,
  getWeightsTotal,
  getSuggestedPlan,
  calculateRebalance,
  normalizeNumber
} from "./state.js";
import {
  getElements,
  setApiBaseUrl,
  setFeedback,
  renderUsers,
  renderIdealPlan,
  renderWeightsHint,
  renderSummary
} from "./ui.js";

const api = new ApiClient(API_BASE_URL);
const elements = getElements();

function applyWeightsOnForm(weights) {
  Object.entries(weights).forEach(([key, value]) => {
    const input = elements.weightsForm.elements.namedItem(key);
    if (input) {
      input.value = String(value);
    }
  });
}

function applyCurrentOnForm(values) {
  Object.entries(values).forEach(([key, value]) => {
    const input = elements.currentForm.elements.namedItem(key);
    if (input) {
      input.value = String(value);
    }
  });
}

async function loadUsers() {
  try {
    setFeedback(elements.feedback, "Carregando usuarios do backend C...", "neutral");
    const users = await api.getUsers();
    state.users = users;
    renderUsers(elements.usersContainer, state.users, handleDeleteUser);
    setFeedback(elements.feedback, "Usuarios carregados com sucesso.", "success");
  } catch (error) {
    setFeedback(elements.feedback, error.message, "error");
  }
}

async function handleCreateUser(event) {
  event.preventDefault();
  const nome = elements.userName.value.trim();

  if (!nome) {
    setFeedback(elements.feedback, "Informe o nome do usuario para cadastrar.", "error");
    return;
  }

  try {
    setFeedback(elements.feedback, "Enviando POST /usuarios...", "neutral");
    await api.createUser({ nome });
    elements.userForm.reset();
    await loadUsers();
    setFeedback(elements.feedback, "Usuario cadastrado e lista atualizada.", "success");
  } catch (error) {
    setFeedback(elements.feedback, error.message, "error");
  }
}

async function handleDeleteUser(id) {
  if (id === undefined || id === null) {
    setFeedback(elements.feedback, "ID invalido para exclusao.", "error");
    return;
  }

  try {
    setFeedback(elements.feedback, `Removendo usuario #${id}...`, "neutral");
    await api.deleteUser(id);
    await loadUsers();
    setFeedback(elements.feedback, `Usuario #${id} removido com sucesso.`, "success");
  } catch (error) {
    setFeedback(elements.feedback, error.message, "error");
  }
}

function handlePlannerSubmit(event) {
  event.preventDefault();
  const name = elements.investorName.value.trim();
  const income = normalizeNumber(elements.monthlyIncome.value);

  if (!name) {
    setFeedback(elements.feedback, "Informe o nome do investidor.", "error");
    return;
  }

  if (!income) {
    setFeedback(elements.feedback, "Informe um ganho mensal valido.", "error");
    return;
  }

  state.monthlyIncome = income;
  state.idealPlan = getSuggestedPlan(income);
  renderIdealPlan(elements.idealPlan, state.idealPlan, income);

  setFeedback(
    elements.feedback,
    `Plano sugerido para ${name}. Se quiser, ajuste os pesos no passo 2.`,
    "success"
  );
}

function handleWeightsSubmit(event) {
  event.preventDefault();
  const parsed = parseAllocationForm(elements.weightsForm);
  const total = getWeightsTotal(parsed);

  state.weights = parsed;
  renderWeightsHint(elements.weightsTotal, total);

  if (Math.abs(total - 100) > 0.0001) {
    setFeedback(elements.feedback, "A soma dos pesos precisa ser 100% para rebalancear corretamente.", "error");
    return;
  }

  setFeedback(elements.feedback, "Pesos da carteira atualizados.", "success");
}

function handleCurrentSubmit(event) {
  event.preventDefault();
  state.currentInvestments = parseAllocationForm(elements.currentForm);
  setFeedback(elements.feedback, "Valores atuais salvos no estado da aplicacao.", "success");
}

function handleRebalance() {
  const totalWeights = getWeightsTotal(state.weights);
  if (Math.abs(totalWeights - 100) > 0.0001) {
    setFeedback(elements.feedback, "Nao e possivel rebalancear com pesos diferentes de 100%.", "error");
    return;
  }

  const monthlyContribution = state.idealPlan?.monthlyContribution ?? state.monthlyIncome * 0.2;
  const summary = calculateRebalance(state.weights, state.currentInvestments, monthlyContribution);
  renderSummary(elements.summary, summary);
  setFeedback(elements.feedback, "Resumo de rebalanceamento calculado.", "success");
}

function bootstrap() {
  setApiBaseUrl(elements.apiBaseUrl, API_BASE_URL);

  applyWeightsOnForm(defaultWeights);
  applyCurrentOnForm(state.currentInvestments);

  renderIdealPlan(elements.idealPlan, null, 0);
  renderWeightsHint(elements.weightsTotal, getWeightsTotal(defaultWeights));
  renderSummary(elements.summary, null);

  elements.userForm.addEventListener("submit", handleCreateUser);
  elements.refreshUsersButton.addEventListener("click", loadUsers);
  elements.plannerForm.addEventListener("submit", handlePlannerSubmit);
  elements.weightsForm.addEventListener("submit", handleWeightsSubmit);
  elements.currentForm.addEventListener("submit", handleCurrentSubmit);
  elements.rebalanceButton.addEventListener("click", handleRebalance);

  loadUsers();
}

bootstrap();
