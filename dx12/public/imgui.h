#pragma once

#include "technique.h"

namespace BNOctaves
{
    inline void ShowToolTip(const char* tooltip)
    {
        if (!tooltip || !tooltip[0])
            return;

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        ImGui::Text("[?]");
        ImGui::PopStyleColor();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("%s", tooltip);
    }

    void MakeUI(Context* context, ID3D12CommandQueue* commandQueue)
    {
        ImGui::PushID("gigi_BNOctaves");

        {
            static const char* labels[] = {
                "Blue",
                "White",
                "Binomial3x3",
                "Box3x3",
                "Box5x5",
                "Perlin",
                "R2",
                "IGN",
                "BlueReverse",
            };
            ImGui::Combo("NoiseType", (int*)&context->m_input.variable_NoiseType, labels, 9);
            ShowToolTip("The type of noise to use");
        }
        {
            int localVar = (int)context->m_input.variable_NumberOfOctaves;
            if(ImGui::InputInt("NumberOfOctaves", &localVar, 0))
                context->m_input.variable_NumberOfOctaves = (unsigned int)localVar;
            ShowToolTip("How many octaves to use");
        }
        ImGui::Checkbox("DifferentNoisePerOctave", &context->m_input.variable_DifferentNoisePerOctave);
        ShowToolTip("If false, the same noise will be used for each octave. If true, a different noise, of the same type, will be used for each octave.");
        {
            int localVar = (int)context->m_input.variable_RNGSeed;
            if(ImGui::InputInt("RNGSeed", &localVar, 0))
                context->m_input.variable_RNGSeed = (unsigned int)localVar;
            ShowToolTip("A PRNG is used for various things, change this value to change thats eed.");
        }
        {
            int localVar = (int)context->m_input.variable_PerlinCellSize;
            if(ImGui::InputInt("PerlinCellSize", &localVar, 0))
                context->m_input.variable_PerlinCellSize = (unsigned int)localVar;
            ShowToolTip("");
        }
        {
            float width = ImGui::GetContentRegionAvail().x / 4.0f;
            ImGui::PushID("PerlinMinMax");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_PerlinMinMax[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_PerlinMinMax[1]);
            ImGui::SameLine();
            ImGui::Text("PerlinMinMax");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("Perlin noise can go below zero which causes problems in this demo. To help that, this is the range of values which are mapped to [0,1]. Anything lower than 0 is clipped to 0 after the remapping.");
        }
        {
            int localVar = (int)context->m_input.variable_BlueReverseStartSize;
            if(ImGui::InputInt("BlueReverseStartSize", &localVar, 0))
                context->m_input.variable_BlueReverseStartSize = (unsigned int)localVar;
            ShowToolTip("");
        }
        {
            int localVar = (int)context->m_input.variable_Histogram_NumBuckets;
            if(ImGui::InputInt("Histogram_NumBuckets", &localVar, 0))
                context->m_input.variable_Histogram_NumBuckets = (unsigned int)localVar;
            ShowToolTip("");
        }
        {
            float width = ImGui::GetContentRegionAvail().x / 4.0f;
            ImGui::PushID("Histogram_GraphSize");
            ImGui::PushItemWidth(width);
            int localVarX = (int)context->m_input.variable_Histogram_GraphSize[0];
            if(ImGui::InputInt("##X", &localVarX, 0))
                context->m_input.variable_Histogram_GraphSize[0] = (unsigned int)localVarX;
            ImGui::SameLine();
            int localVarY = (int)context->m_input.variable_Histogram_GraphSize[1];
            if(ImGui::InputInt("##Y", &localVarY, 0))
                context->m_input.variable_Histogram_GraphSize[1] = (unsigned int)localVarY;
            ImGui::SameLine();
            ImGui::Text("Histogram_GraphSize");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }
        {
            float width = ImGui::GetContentRegionAvail().x / 4.0f;
            ImGui::PushID("Histogram_XAxisRange");
            ImGui::PushItemWidth(width);
            ImGui::InputFloat("##X", &context->m_input.variable_Histogram_XAxisRange[0]);
            ImGui::SameLine();
            ImGui::InputFloat("##Y", &context->m_input.variable_Histogram_XAxisRange[1]);
            ImGui::SameLine();
            ImGui::Text("Histogram_XAxisRange");
            ImGui::PopItemWidth();
            ImGui::PopID();
            ShowToolTip("");
        }
        ImGui::Checkbox("Histogram_AutoXAxisRange", &context->m_input.variable_Histogram_AutoXAxisRange);
        ShowToolTip("");
        ImGui::Checkbox("Histogram_ZeroMinMaxBucket", &context->m_input.variable_Histogram_ZeroMinMaxBucket);
        ShowToolTip("If values are clamped to a min and max value, the min and max bucket will have too many counts in them. This option zeros them out to make the rest of the data easier to see.");
        ImGui::Checkbox("DFT_RemoveDC", &context->m_input.variable_DFT_RemoveDC);
        ShowToolTip("DC (0hz) is often a large spike that makes it hard to see the rest of the frequencies. Use this to set DC to zero.");
        ImGui::Checkbox("DFT_LogSpaceMagnitude", &context->m_input.variable_DFT_LogSpaceMagnitude);
        ShowToolTip("If true, show magnitude in log space");

        ImGui::Checkbox("Profile", &context->m_profile);
        if (context->m_profile)
        {
            int numEntries = 0;
            const ProfileEntry* entries = context->ReadbackProfileData(commandQueue, numEntries);
            if (ImGui::BeginTable("profiling", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Label");
                ImGui::TableSetupColumn("CPU ms");
                ImGui::TableSetupColumn("GPU ms");
                ImGui::TableHeadersRow();
                float totalCpu = 0.0f;
                float totalGpu = 0.0f;
                for (int entryIndex = 0; entryIndex < numEntries; ++entryIndex)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::TextUnformatted(entries[entryIndex].m_label);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3f", entries[entryIndex].m_cpu * 1000.0f);
                    ImGui::TableNextColumn();
                    ImGui::Text("%0.3f", entries[entryIndex].m_gpu * 1000.0f);
                    totalCpu += entries[entryIndex].m_cpu;
                    totalGpu += entries[entryIndex].m_gpu;
                }
                ImGui::EndTable();
            }
        }

        ImGui::PopID();
    }
};
